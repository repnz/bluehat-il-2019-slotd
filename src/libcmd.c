//
//
// This module manages the command line interface of the program
//
// The command interface loads command by using shared objects.
// The commands are loaded from ./libcmd_{cmd}.so.
// After a command is loaded, the exported cmd_main() function is called
// Handles to loaded commands are cached in libkv store 'cmd' under the command name.
//
// The supported commands are:
// - cat
// - echo
// - help
// - ip
// - kv_dump
// - ls
// - mount
// - prompt
// - startup
// - user

#include "libcmd.h"
#include "libcfg.h"
#include "libkv.h"
#include "user.h"
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


// Check that the command is valid
// only chars from 'abcdefghijklmnopqrstuvwxyz-' are valid
// Return true if it is valid else false
static int is_valid_cmd_str(const char *str)
{
    return strspn(str, "abcdefghijklmnopqrstuvwxyz-") == strlen(str);
}

//
// Get the filename from the command name
// ./libcmd_{cmd}.so 
// Returns:
//      malloc() filename pointer if it is valid - has to be freed
//      NULL if the cmd is not valid or asprintf could not allocate 
static char *cmd_str_to_filename(const char *str)
{
    char *result;
    char *cp;

    // check if the command name is valid
    if (!is_valid_cmd_str(str)) {
        return NULL;
    }

    // create the command string
    // asprintf allocates the string 
    if (asprintf(&result, "./libcmd_%s.so", str) < 0) {
        return NULL;
    }

    //
    // replace any '-' with _
    // 
    cp = result;
    while ((cp = strchr(cp, '-')) != NULL) {
        *cp = '_';
    }

    return result;
}

// Call dlclose basically
// If the handle is NULL, ignore
static void unload_cmd(void *handle)
{
    if (handle) {
        dlclose(handle);
    }
}

//
// This function loads a command plugin
// Store a cached value in libkv under 'cmd'
//
// Returns
//      NULL in case of failure:
//          - invalid command 
//          - kv_ref returned NULL
//      
static void *load_cmd(const char *str)
{
    char *filename;
    void **cmd_ref;

    // check that str is valid (again..)
    if (!is_valid_cmd_str(str)) {
        return NULL;
    }

    // Uses kv to store the loaded commands
    // Get the command from the 'cmd' store
    cmd_ref = kv_ref("cmd", str, unload_cmd);

    if (!cmd_ref) {
        return NULL;
    }

    // If the command is already loaded,
    // return the command handle
    if (*cmd_ref) {
        return *cmd_ref;
    }

    // Get the filename of the command file
    // replace any - with _
    filename = cmd_str_to_filename(str);
    if (!filename) {
        return NULL;
    }

    // Load the dynamic library. 
    *cmd_ref = dlopen(filename, RTLD_LOCAL | RTLD_NOW);
    free(filename);
    return *cmd_ref;
}

//
// Execute a command.
// Uses dlsym to load 'cmd_main' from the command an runs it
//
// Arguments:
// cmd: a command module handle
// param: the parameter to pass to the command
//
// Returns:
//      -1 if 'cmd_main' could not be found
//      0 if the command has run successfully
//
static int exec_cmd(void *cmd, char *param)
{
    void (*cmd_main)(char *param);

    cmd_main = (void (*)(char *))dlsym(cmd, "cmd_main");
    if (!cmd_main) {
        return -1;
    }

    cmd_main(param);
    return 0;
}

// Print the prompt from env:PS1 
// If env:PS1 does not exist, print '> '
// flush to stdout
static void print_prompt(void)
{
    const char *prompt;

    prompt = kv_get("env", "PS1");
    printf("%s", prompt ? prompt : "> ");
    fflush(stdout);
}

// Split the user input to the command and param
// The input has to be like this:
// [COMMAND](delimeter)[PARAMETER CAN BE MULTIPILE..]
// Puts \0 between the command and the param in str
//
// Returns the param of the command
//    empty for an empty string
//    empty if the param is empty
//
static char *split_input(char *str)
{
    // Length of the buffer string
    size_t len = strlen(str);

    // Length of the command part inside the buffer
    size_t cmd_len;
    
    // The delimeters of the buffer
    static const char *const delimiters = " \t\n";
    
    // The parameter. 
    char *param;

    if (len == 0) {
        // Empty string
        return str;
    }

    // If \n is found at the end, replace it with \0
    if (str[len - 1] == '\n') {
        len--;
        str[len] = '\0';
    }

    // Get The end of the command buffer (including param)
    // Put \0 intead of the first \t or \n or space
    cmd_len = strcspn(str, delimiters);
    str[cmd_len] = '\0';

    if (cmd_len == len) {
        // Return the end of the buffer
        // Returns \0 because there is no param
        return str + cmd_len;
    }

    // get the param
    param = str + cmd_len + 1;

    // Return the param. Skip any delimeter at the beginning
    return param + strspn(param, delimiters);
}

// Check if the command is an exit command
// str == exit || str == quit
static int is_exit_command(const char *cmd)
{
    return (strcmp(cmd, "exit") == 0) || (strcmp(cmd, "quit") == 0);
}

// 
// Parse & Execute one line from the script
// Returns:
//      -1 in case of an error: 
//          - empty buffer
//          - invalid chars inside the command
//
//      1 in case of an exit command or an error finding 'cmd_main' in the command object
//      0 in case of success
//
static int do_line(char *buffer)
{
    char *param;
    void *cmd;

    // Split the input and get the parameter
    // command is divided like this:
    // [COMMAND] [PARAMETER CAN BE MULTIPILE..]
    // buffer is changed to be only the command
    // param is the parameter to the command
    param = split_input(buffer);

    // If the buffer or param is empty return zero
    if (buffer[0] == '\0' && param[0] == '\0') {
        return -1;
    }

    // The buffer is checked to be valid
    // param is not checked
    if (!is_valid_cmd_str(buffer)) {
        fprintf(stderr, "bad command %s\n", buffer);
        return -1;
    }

    // Check if the cmd is quit or exit
    if (is_exit_command(buffer)) {
        return 1;
    }

    // Load the command
    // PROBLEM: even if the library could not be loaded,
    // the name is cached in libkv.. is it a problem?
    cmd = load_cmd(buffer);
    if (!cmd) {
        fprintf(stderr, "unknown command %s\n", buffer);
        return -1;
    }

    // Finally execute the command
    if (exec_cmd(cmd, param) < 0) {
        fprintf(stderr, "internal error in command %s\n", buffer);
        return 1;
    }

    return 0;
}


//
// Run a script
// Used to run the login script created in main()
// Returns:
//      -1 in case of an error:
//          - if the script is empty
//          - if the strdup() did not work
//      0 in case of success
//
static int run_script(const char *script)
{
    char *buf, *line, *saveptr;

    // if the script is empty, return -1
    if (!script) {
        return -1;
    }

    // duplicate the script string
    // Probably because using strtok_r to modify the string
    buf = strdup(script);

    if (!buf) {
        return -1;
    }

    // Iterate each line of the script
    for (line = strtok_r(buf, "\n", &saveptr);
         line;
         line = strtok_r(NULL, "\n", &saveptr)) {
        // Execute the line.
        // If the result is 1, it means the script ends
        if (do_line(line) == 1) {
            break;
        }
    }

    return 0;
}

//
// Performs an initialization of the current logged in session
// Creates an 'env' store and stores there: 
// - 'USER' -> the logged in username (copied with strdup)
// - 'PS1' -> the char to print to the command line
//
static int loop_setup(const struct user *user)
{
    char **env_ref;

    // defines a simple key-value struct
    // then uses this struct to define an array 
    //
    // This array contains:
    // USER: specifies the logged in user
    // PS1: specifies the char printed on the command line
    struct var {

        const char *key;
        const char *value;

    } vars[] = {
        {"USER", user->username},
        {"PS1", "> "},
        {NULL, NULL},
    };
    
    // Iterate each key-value in this array
    for (const struct var *var = vars; var->key; var++) {

        // Insert this key-value into 'env' store
        // The 'env' store represents the current environment
        env_ref = kv_ref("env", var->key, free);
        if (!env_ref) {
            return -1;
        }

        // If there was any value before, free it
        if (*env_ref) {
            free(*env_ref);
        }

        // Duplicate the value and store it 
        *env_ref = strdup(var->value);
        if (!*env_ref) {
            return -1;
        }
    }

    // Run the script in the startup
    run_script(cfg_get("startup"));

    return 0;
}

static void loop_teardown(void)
{
    // Remove cmd and env
    kv_flush("cmd");
    kv_flush("env");
}


//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------
// ------------------------------------- API of the module is defined below --------------------------------
//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------




//
// After a successful login, the code reaches here
// This is the main loop of the program.
// 
// Arguments:
// user: the pointer to the user that has logged in
//
// 
void cmd_loop(const struct user *user)
{
    char *buffer = NULL;
    size_t bufsz = 0;
    ssize_t len;

    // Initialize the login session
    if (loop_setup(user) < 0) {
        fprintf(stderr, "init failed\n");
        loop_teardown();
        return;
    }

    // Session loop
    while (1) {

        // print >
        print_prompt();

        // get user input
        len = getline(&buffer, &bufsz, stdin);

        // if user input is incorrect break
        if (len <= 0) {
            break;
        }

        // parse and run user input
        if (do_line(buffer) == 1) {
            break;
        }
    }

    // getline() allocates the memory at the first time it is called,
    // After that the same memory chunk is used.
    // What if the first chunk of memory is less then the size of the buffer?
    // If the buffer is not large enough it is resized using realloc
    // So the memory management here is fine. (or so it seems)
    free(buffer);

    loop_teardown();
}
