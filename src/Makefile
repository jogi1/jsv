CC=clang
EXE=jsv

INCPATH= \
	-I/usr/pkg/include

LIBPATH= \
	-L/usr/pkg/lib

SRC= \
	server.c \
	log.c \
	info.c \
	tools.c \
	net.c \
	packet.c \
	tokenize_string.c \
	cmd.c \
	lua.c \
	model.c \
	md4.c \
	vector.c \
	world.c \
	trace.c \
	physics.c

OBJ=$(SRC:.c=.o)

# if debugging
CFLAGS+= -g
CFLAGS+= -D__LITTLE_ENDIAN__Q__
CFLAGS+= -Wall
CFLAGS+= -O0

TARGETSYSTEM:=$(shell gcc -dumpmachine)

OS=$(shell echo $(TARGETSYSTEM) | sed "s/-gnu//" | sed "s/.*-//" | tr [A-Z] [a-z] | sed s/^mingw.*/win32/ | sed s/^openbsd.*/openbsd/ | sed s/^freebsd.*/freebsd/ | sed s/^netbsd.*/netbsd/ | sed s/^darwin.*/macosx/)
BIT=$(shell echo $(TARGETSYSTEM) | sed "s/.*64.*/64/")

ifeq ($(BIT),64)
	CFLAGS+=-m64
endif

ifeq ($(OS),linux)
	LFLAGS+=-lrt
	INCPATH+=$(shell pkg-config --cflags lua5.1)
	LIBS+=$(shell pkg-config --libs lua5.1)
endif

ifeq ($(OS),freebsd)
	LFLAGS+=-lm -lpthread
	CFLAGS+=-D__FIX_THIS_WEIRD_BUG__
	INCPATH+=$(shell pkg-config --cflags lua-5.1)
	LIBS+=$(shell pkg-config --libs lua-5.1)
endif

ifeq ($(OS),netbsd)
	LFLAGS+=-pthread
	CFLAGS+=-D__FIX_THIS_WEIRD_BUG__
	INCPATH+=$(shell pkg-config --cflags lua)
	LIBS+=$(shell pkg-config --libs lua)
endif

ifeq ($(OS),darwin)
	echo "meow"
endif

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(LFLAGS) $(INCPATH) $(LIBPATH) $(LIBS) $(OBJ) -o $(EXE)

.c.o:
	$(CC) $(CFLAGS) $(INCPATH) -c $< -o $@

clean:
	rm -f $(EXE) *.o


