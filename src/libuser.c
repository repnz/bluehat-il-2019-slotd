#include "libuser.h"
#include "user.h"
#include "libkv.h"
#include <string.h>
#include <stdlib.h>

static struct user *alloc_user(void)
{
    struct user *user;

    user = calloc(1, sizeof(*user));
    if (!user) {
        return NULL;
    }

    return user;
}

static void user_dtor(struct user *user)
{
    free(user);
}

static int user_add_or_set(const char *username, const char *password)
{
    struct user **user_ref;
    
    if (strlen(username) >= sizeof((*user_ref)->username)) {
        return -1;
    }

    if (strlen(password) >= sizeof((*user_ref)->password)) {
        return -1;
    }

    user_ref = kv_ref("user", username, user_dtor);
    if (!user_ref) {
        return -1;
    }

    if (!*user_ref) {
        *user_ref = alloc_user();
        if (!*user_ref) {
            kv_del("user", username);
            return -1;
        }
    }

    strncpy((*user_ref)->username, username, sizeof((*user_ref)->username) - 1);
    strncpy((*user_ref)->password, password, sizeof((*user_ref)->password) - 1);

    return 0;
}

int user_add(const char *username, const char *password)
{
    if (kv_in("user", username)) {
        return -1;
    }

    return user_add_or_set(username, password);
}

int user_set(const char *username, const char *password)
{
    if (!kv_in("user", username)) {
        return -1;
    }

    return user_add_or_set(username, password);
}

const struct user *user_get(const char *username)
{
    return kv_get("user", username);
}

int user_del(const char *username)
{
    return kv_del("user", username);
}

const char *user_current(void)
{
    return kv_get("env", "USER");
}
