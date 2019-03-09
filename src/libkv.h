//
// An implementation of a simple key value store
// Used for:
// 1) Storing configuration
//
#ifndef __LIBKV_H__
#define __LIBKV_H__

void **kv_ref(const char *storename, const char *key, void (*dtor)(void *));
void *kv_get(const char *storename, const char *key);
int kv_in(const char *storename, const char *key);
int kv_iter(const char *storename, void (*func)(const char *key, void **value));
int kv_del(const char *storename, const char *key);
int kv_flush(const char *storename);
void kv_clear();

#endif /* __LIBKV_H__ */
