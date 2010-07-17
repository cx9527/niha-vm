CC=gcc
RM=rm
CFLAGS=-W -Wall -Werror -Wuninitialized -O2 -pedantic -g
LDFLAGS=

#SRCS=virtualmachine.c compilo.c tools.c
#OBJS=$(SRCS:.c=.o)
LIBS=
MAIN=virtualmachine compilo

.SUFFIXES:
.SUFFIXES: .o .c

all: $(MAIN)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

#$(MAIN): $(OBJS)
#	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(OBJS) -o $(MAIN)

virtualmachine: tools.o virtualmachine.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $^ -o $@

compilo: tools.o compilo.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $^ -o $@

clean:
	$(RM) -rf *.o *.bc $(MAIN)
