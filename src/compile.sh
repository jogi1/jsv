#!/bin/bash
clang -g server.c world.c log.c info.c tools.c net.c packet.c tokenize_string.c cmd.c lua.c model.c md4.c vector.c physics.c -D__LITTLE_ENDIAN__Q__ -I/usr/include/lua5.1/ -llua5.1 -Wall -lrt
