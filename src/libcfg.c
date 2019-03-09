// libcfg: Configuration module. 
// Uses the libkv module to store key-value pairs in the 'cfg' store
//
#include "libkv.h"
#include <stdlib.h>
#include <string.h>

// Set configuration property
// var: the key name. const c string
// val: the name of the value to store. const c string 
//
// return -1 in case:
// 1) if the pair could not be created 
// 2) if the string could not be duplicated
int cfg_set(const char *var, const char *val)
{
    char **ref;

    // create a key inside the store cfg
    // the name of the key is var
    // the dtor is free
    // BUG: maybe free is used to free the const object?
    ref = kv_ref("cfg", var, free);

    // if the pair could not be created
    if (!ref) {
        return -1;
    }

    // if there is value inside already, free it
    if (*ref) {
        free(*ref);
    }

    // duplicate the val pointer 
    *ref = strdup(val);

    // if we could not duplicate the string return -1
    if (!*ref) {
        return -1;
    }

    return 0;
}

// Get some configuration property
// var: the key name
// returns the value of that variable
const char *cfg_get(const char *var)
{
    return kv_get("cfg", var);
}
