CFILES=$(wildcard src/base/*.c)\
	$(wildcard src/tools/*.c)

obj=$(CFILES:.c=.o)

CFLAGS=-g -I./src/base/ -I./src/tools/
CC=gcc

comp: $(obj)
	$(CC) -o $@ $^ $(CFLAGS)

