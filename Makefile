CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC -std=c11
LDFLAGS = -pthread
LIB_NAME = libivm.so 

SRCS = main.c IntMemoryManager.c ivm-api.c
OBJS = $(SRCS:.c=.o)
PREFIX = /usr/local

install: $(LIB_NAME)
	install -d $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include/ivm
	
	install -m 755 $(LIB_NAME) $(DESTDIR)$(PREFIX)/lib/
	
	install -m 644 ivm.h $(DESTDIR)$(PREFIX)/include/ivm/
	install -m 644 IntMemoryManager.h $(DESTDIR)$(PREFIX)/include/ivm/
	
	ldconfig
	@echo "\033[32m[INSTALLED]\033[0m IVM."

all: $(LIB_NAME)

$(LIB_NAME): $(OBJS)
	$(CC) -shared -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(LIB_NAME)
