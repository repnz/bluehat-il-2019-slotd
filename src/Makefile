CFLAGS=-std=gnu99 -Wall -Wextra -Wno-unused-parameter -Wno-incompatible-pointer-types -Werror -fstack-protector-all -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE -O2
LDFLAGS=-Wl,-z,now -Wl,-z,relro -L.

libs=$(wildcard lib*.c)
targets=slotd jackpot $(patsubst %.c,%.so,$(libs))

.PHONY=all
all: $(targets)

slotd: slotd.c libkv.so liblogin.so libcmd.so libcfg.so
	$(CC) $(CFLAGS) $(LDFLAGS) -pie -fpie $< -o $@ -lkv -llogin -lcmd -lcfg

jackpot: jackpot.c
	$(CC) $(CFLAGS) $(LDFLAGS) -static -fpie $< -o $@

define buildlib
lib$1.so: lib$1.c $(patsubst %,lib%.so,$2)
	$$(CC) $$(CFLAGS) $$(LDFLAGS) $$< -shared -fpic -o $$@ $$(patsubst %,-l%,$2 $3)
endef

$(eval $(call buildlib,kv,))
$(eval $(call buildlib,login,cfg kv,))
$(eval $(call buildlib,user,kv,))
$(eval $(call buildlib,cmd,kv cfg,dl))
$(eval $(call buildlib,sh,,,))
$(eval $(call buildlib,cfg,kv,))

$(eval $(call buildlib,cmd_help,,))
$(eval $(call buildlib,cmd_echo,,))
$(eval $(call buildlib,cmd_cat,,))
$(eval $(call buildlib,cmd_user,user,))
$(eval $(call buildlib,cmd_prompt,kv user,))
$(eval $(call buildlib,cmd_ls,sh,))
$(eval $(call buildlib,cmd_mount,sh,))
$(eval $(call buildlib,cmd_startup,cfg,))
$(eval $(call buildlib,cmd_kv_dump,kv,))
$(eval $(call buildlib,cmd_ip,sh,))

.PHONY=clean
clean:
	rm -f $(targets)
