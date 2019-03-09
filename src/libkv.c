//
// libkv - create and store named dictionaries
//
// This module allows defining dictionaries by name (that are called 'stores')
// And using the stores to define key-value pairs.
// For example, the cfg module uses this module to store configuration in the 
// 'cfg' store.
//  
// Used in: libcfg
// Uses: nothing
// 
// Implementation: Every store is a linked list of key-value pairs.

// Stores::
//
// cfg:
// - login_msg: const char* msg. 
//      slotd!main
// - startup: const char* msg: script that is run 
//      slotd!main
//
//
// login:
// - debug
//      main() --> login_do() --> login_enable(static { "debug", "debug"})
//
// env: - the current session. created in loop_setup
// - USER - the logged in username
// - PS1  - the prompt char of the shell
//
// cmd: 
// - {cmd} - handle to a cached command. if the command could not be loaded, it sets the value to zero
//
// _kv: 
// - {storename} - {pointer to the store object} (libkv!find_create_store())
//
// a value can never be NULL (but it can!)
//

#include "libkv.h"
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h> // Used for the linked lists
#include <assert.h>


// A struct representing a key-value pair
struct item {
    LIST_ENTRY(item) list;
    char *key;
    void *value;
    void (*dtor)(void *);
};

 // the store struct is a head of a linked list of items
struct store {
    LIST_HEAD(itemlist, item) list;
};

// The initial store.
static struct store initial_store = {
    .list = LIST_HEAD_INITIALIZER(list)
};


//
// Allocate the store
// Use calloc() to allocate the store
// the store struct is a head of a linked list of items
// Returns:
//      NULL if the allocation has failed
//      pointer to the store if it succeeded
//
static struct store *alloc_store(void)
{
    struct store *store;
    store = calloc(1, sizeof(*store));
    if (!store) {
        return NULL;
    }

    LIST_INIT(&store->list);
    return store;
}


// Free the store object from memory
static void free_store(struct store *store)
{
    // check that the list is empty
    assert(LIST_EMPTY(&store->list));

    // free the object!
    free(store);
}


//
// Allocate memory for the item 
// item {
//      .key = strdup(key),
//      .value = NULL, 
//      .dtor = NULL 
// }
// key is copied with strdup into the key
// Returns NULL if:
//      - the item could not be allocated
//      - strdup() could not copy the item
//      - 
static struct item *alloc_item(const char *key)
{
    // the allocated item
    struct item *item;

    // allocate item
    item = calloc(1, sizeof(*item));
    if (!item) {
        return NULL;
    }

    // copy the key into the item with strdup
    item->key = strdup(key);

    // if strdup did not work
    // free the item and return NULL
    if (!item->key) {
        free(item);
        return NULL;
    }

    item->value = NULL;
    item->dtor = NULL;

    return item;
}


// Free an item from memory
// Frees item->key and then the item
static void free_item(struct item *item)
{
    assert(item->value == NULL);
    assert(item->dtor == NULL);
    free(item->key);
    free(item);
}


//
// Deletes the item 
// If dtor exists for the value, call the dtor
//
static void del_item(struct item *item)
{
    // Remove the item from the list
    LIST_REMOVE(item, list);

    // call dtor
    if (item->dtor) {
        item->dtor(item->value);
        item->dtor = NULL;
    }

    item->value = NULL;

    // free the item struct from memory
    free_item(item);
}


// Find the item object in the store
// Linear search for the key name
// if the item is found, a pointer is returned
// if not, NULL is returned
static struct item *find_item(struct store *store, const char *key)
{
    struct item *item;

    LIST_FOREACH(item, &store->list, list) {
        if (strcmp(item->key, key) == 0) {
            return item;
        }
    }

    return NULL;
}


//
// Find an item object in the store
// If the item does not exist, it is allocated and inserted to the head of the list
// Returns NULL if the item could not be allocated
static struct item *find_create_item(struct store *store, const char *key)
{   
    // find item
    struct item *item = find_item(store, key);

    // if the item does not exist
    if (!item) {
        // it is allocated
        item = alloc_item(key);
        if (!item) {
            return NULL;
        }

        // item is inserted to the head of the list
        LIST_INSERT_HEAD(&store->list, item, list);
    }

    return item;
}


// Delete all the items in the store
static void kv_store_delall(struct store *store)
{
    struct item *item;

    while (!LIST_EMPTY(&store->list)) {
        item = LIST_FIRST(&store->list);
        del_item(item);
    }
}

//
// Destruct the store object
// Call the dtor for all of the stored objects
// free the store object from memory
static void kv_store_dtor(struct store *store)
{
    kv_store_delall(store);
    free_store(store);
    return;
}

//
// Finds a pointer to a store 
// If the store does not exist it creates it
// If the name of the store is _kv, the first store is returned
// Returns:
//      NULL if the key inside _kv could not be created
//      NULL if the store could not be allocated with alloc_store()
//
//
static struct store *find_create_store(const char *storename)
{
    struct store **store_ref;

    if (strcmp("_kv", storename) == 0) {
        return &initial_store;
    }

    // a reference to a key inside _kv is taken..
    store_ref = kv_ref("_kv", storename, kv_store_dtor);
    if (!store_ref) {
        return NULL;
    }

    // if the store does not exist
    if (!*store_ref) {
        /* first ref, alloc it */ // original comment. was in the source
        // store a pointer to the store under _kv
        *store_ref = alloc_store();
        if (!*store_ref) {
            return NULL;
        }
    }

    return *store_ref;
}



//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------
// ------------------------------------- API of the module is defined below --------------------------------
//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------


//
// Get a pointer to a new/existing key-value in a new/existing key in this store
//
// Arguments:
//      storename: the name of the store
//      key: the name of the key
//
// Returns:
//      NULL in case of an error
//          - a store could not be created
//          - key could not be created
//
//  
void **kv_ref(const char *storename, const char *key, void (*dtor)(void *))
{
    struct store *store;
    struct item *item;

    // Get the store 
    // If the store does not exist it creates it
    store = find_create_store(storename);
    if (!store) {
        return NULL;
    }

    // Get the key from the store
    // If it does not exist it creates it
    item = find_create_item(store, key);
    if (!item) {
        return NULL;
    }

    // set the dtor
    item->dtor = dtor;
    return &(item->value);
}

//
// Get the actual value of that key
// If the store does not exist it creates it
// If the key does not exist, return NULL
// So, in case the store is created, the key is always NULL
// Return the value in that key in the form of (void*)
void *kv_get(const char *storename, const char *key)
{
    struct store *store;
    struct item *item;

    store = find_create_store(storename);
    if (!store) {
        return NULL;
    }

    item = find_item(store, key);
    if (!item) {
        return NULL;
    }
    
    return item->value;
}


//
// Check if a certain key exist in this store
// Creates the store if it does not exist
int kv_in(const char *storename, const char *key)
{
    struct store *store;

    store = find_create_store(storename);
    if (!store) {
        return 0;
    }

    return find_item(store, key) != NULL;
}


//
// iterate the pairs inside a store
// If the store does not exist it creates it 
// the function 'func' is called for each key in the store
// Returns -1 if the store could not be created
int kv_iter(const char *storename, void (*func)(const char *key, void **value))
{
    struct store *store;
    struct item *item;

    store = find_create_store(storename);
    if (!store) {
        return -1;
    }

    LIST_FOREACH(item, &store->list, list) {
        func(item->key, &item->value);
    }

    return 0;
}

//
// Delete a key in a store
// If the store does not exist it creates it
// return NULL if the key does not exist
// return 0 if the item has been deleted
int kv_del(const char *storename, const char *key)
{
    struct store *store;
    struct item *item;
    
    store = find_create_store(storename);
    if (!store) {
        return -1;
    }

    item = find_item(store, key);
    if (!item) {
        return -1;
    }

    del_item(item);
    return 0;
}


// Delete a key name under _kv named {storename}
int kv_flush(const char *storename)
{
    return kv_del("_kv", storename);
}

// Clear all the stores
void kv_clear(void)
{
    kv_store_delall(&initial_store);
}
