SRC = main.c
HEAD = ivm.h
CC_FLAGS = -Wall -Wextra -g -std=c11
CC = gcc

all:
	${CC} ${SRC} ${HEAD	} ${CC_FLAGS} -o ivm
