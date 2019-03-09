#ifndef __LIBLOGIN_H__
#define __LIBLOGIN_H__

#include "user.h"

const struct user *login_do(void);

int login_enable(const struct user *user);
void login_disable(const struct user *user);

#endif /* __LIBLOGIN_H__ */
