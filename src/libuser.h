#ifndef __LIBUSER_H__
#define __LIBUSER_H__

#include "user.h"

int user_add(const char *username, const char *password);
int user_set(const char *username, const char *password);
const struct user *user_get(const char *username);
int user_del(const char *username);
const char *user_current(void);

#endif /* __LIBUSER_H__ */
