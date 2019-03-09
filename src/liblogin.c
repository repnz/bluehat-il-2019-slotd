#include "liblogin.h"
#include "libcfg.h"
#include "libkv.h"
#include "user.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// Read buffer from the remote user
// printf the prompt before reading input
// Returns:
//    NULL in case of an error,
//    char* -> the user input, has to be freed 
//             this buffer is allocated with getline()
//             NOTE: the length of the buffer is unlimited!!
// 
static char *read_buf(const char *prompt)
{
    char *buffer = NULL;
    size_t bufsz = 0;
    ssize_t len;

    // print prompt
    printf("%s", prompt);
    fflush(stdout);

    // Uses the getline function from std
    // The buffer has to freed 
    // The data is unlimited ?!
    //
    len = getline(&buffer, &bufsz, stdin);

    // If the line could not be read
    if (len < 0) {
        free(buffer);
        return NULL;
    }

    // replace \n at the end with \0
    // 
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }

    return buffer;
}

//
// Check if the user is authorized to login 
// Check the login store to see if that user is registered
// Returns:
//      NULL if the login failed
//      the user pointer if the login was successfull
//
static const struct user *auth(const char *username, const char *password)
{   
    const struct user *user;

    // Get the value inside login->{username}
    user = kv_get("login", username);


    if (!user) {
        return NULL;
    }

    // If the password is the same
    if (strcmp(user->password, password) != 0) {
        return NULL;
    }

    return user;
}


//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------
// ------------------------------------- API of the module is defined below --------------------------------
//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------


//
// Perform the login operation. 
// Returns 
//      NULL if the login failed 
//      a pointer to the user object if the login was successful
// 
//
const struct user *login_do(void)
{
    // The correct credentials
    static const struct user debug = {"debug", "debug"};

    // pointers before copying into user struct
    char *username, *password;

    // The input user credentials
    const struct user *user = NULL;

    // pointer for messages
    const char *msg;

    // Enable this user to log in
    if (login_enable(&debug) < 0) {
        return NULL;
    }

    // Print the login message
    msg = cfg_get("login_msg");
    if (msg) {
        puts(msg);
    }

    while (!user) {
        // NOTE: The username buffer length is unlimited!
        username = read_buf("Login: ");

        if (!username) {
            return NULL;
        }

        // NOTE: the password buffer length is unlimited!
        password = read_buf("Password: ");

        if (!password) {
            free(username);
            return NULL;
        }

        // Check if the user is authorized to login
        user = auth(username, password);
        
        if (!user) {
            puts("Login failed");
            puts("");
        }

        free(username);
        username = NULL;
        free(password);
        password = NULL;
    }

    return user;
}

// 
// Enable this user to login.
// Stores a pointer to this user object inside libkv
// BUG: maybe dangling user pointer? 
// Returns -1 if kv_ref failed
int login_enable(const struct user *user)
{
    const struct user **user_ref;
    
    // Create a login store in libkv 
    // the dtor of the username is NULL
    user_ref = kv_ref("login", user->username, NULL);


    // if creating the user in libkv failed
    if (!user_ref) {
        return -1;
    }

    // Assign the user pointer to the store
    // Meaning that that the user object lives inside libkv['login']
    *user_ref = user;
    return 0;
}

//
// Delete this user from the login store
// Meaning this user cannot login
//
void login_disable(const struct user *user)
{
    kv_del("login", user->username);
}
