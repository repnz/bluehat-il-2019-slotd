#ifndef __USER_H__
#define __USER_H__

// 
// A struct that contains the input the user inserted
// This is used in the liblogin module
// Beware: the buffers maximum is 256 bytes
struct user {
    char username[256];
    char password[256];
};

#endif /* __USER_H__ */
