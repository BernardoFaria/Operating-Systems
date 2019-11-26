# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2019-20

SOURCES = main.c fs.c sync.c tecnicofs-client-api.c
SOURCES+= lib/bst.c lib/hash.c									# Adicionado lib/hash.c
# OBJS_NOSYNC = $(SOURCES:%.c=%.o)
# OBJS_MUTEX  = $(SOURCES:%.c=%-mutex.o)
OBJS_RWLOCK = $(SOURCES:%.c=%-rwlock.o)
OBJS = $(OBJS_RWLOCK) #$(OBJS_NOSYNC) $(OBJS_MUTEX)
CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../ -g
LDFLAGS=-lm -pthread
TARGETS = tecnicofs-rwlock

.PHONY: all clean

all: $(TARGETS)

$(TARGETS):
	$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)

### RWLOCK ###
lib/hash-rwlock.o: CFLAGS+=-DRWLOCK									
lib/hash-rwlock.o: lib/hash.c lib/hash.h							

lib/bst-rwlock.o: CFLAGS+=-DRWLOCK
lib/bst-rwlock.o: lib/bst.c lib/bst.h

fs-rwlock.o: CFLAGS+=-DRWLOCK
fs-rwlock.o: fs.c fs.h lib/bst.h

sync-rwlock.o: CFLAGS+=-DRWLOCK
sync-rwlock.o: sync.c sync.h constants.h

tecnicofs-client-api.o: CFLAGS+=-DRWLOCK
tecnicofs-client-api.o: tecnicofs-client-api.c tecnicofs-client-api.h

main-rwlock.o: CFLAGS+=-DRWLOCK
main-rwlock.o: main.c fs.h lib/hash.h lib/bst.h constants.h lib/timer.h sync.h tecnicofs-client-api.h
tecnicofs-rwlock: lib/hash-rwlock.o lib/bst-rwlock.o fs-rwlock.o sync-rwlock.o tecnicofs-client-api.o main-rwlock.o	


%.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo Cleaning...
	rm -f $(OBJS) $(TARGETS)
