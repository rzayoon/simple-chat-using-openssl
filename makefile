CC = gcc
CFLAGS = -g -Wall -pthread
LFLAGS = -lssl -lcrypto
RM = /bin/rm
CAT = /bin/cat
OPENSSL = /usr/bin/openssl

OBJS = reentrant.o \
	   common.o	\
	   client.o \
	   server.o

BINS = client server

all: $(BINS)

$(BINS): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(@).o common.o reentrant.o $(LFLAGS)

$(OBJS): common.h reentrant.h

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -f $(BINS) $(OBJS) *~

distclean : clean
