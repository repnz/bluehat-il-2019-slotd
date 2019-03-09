// This module is used to print stores
// can be used to create any store
// If the param is empty, print all the stores
// If the param is used print the store with the name of the param
#include "libkv.h"
#include <stdio.h>

// print a pair of key-value. prints a pointer to the value and the value
static void print_kv(const char *key, void **value)
{
    printf("  %s %p %p\n", key, value, *value);
}

// print the name of the store and all the keys inside the store
static void print_store(const char *store)
{
    printf("%s:\n", store);

    // print all the keys in the store
    kv_iter(store, print_kv);
}

// used with kv_iter to print all the key-values in a store
static void print_root(const char *key, void **value)
{
    print_store(key);
    puts("");
}

void cmd_main(const char *param)
{
    // print the all the stores
    if (param[0] == '\0') {
        kv_iter("_kv", print_root);
        return;
    }

    // print the selected store
    print_store(param);
}
