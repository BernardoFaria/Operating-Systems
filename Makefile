# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2019-20

CC   = gcc
LD   = gcc
CFLAGS =-g -pthread -Wall -std=gnu99 -I../
LDFLAGS=-lm

# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean run

all: tecnicofs-nosync tecnicofs-mutex tecnicofs-rwlock

tecnicofs-nosync: lib/bst.o fs.nosync.o main.nosync.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs-nosync lib/bst.o fs.nosync.o main.nosync.o

tecnicofs-mutex: lib/bst.o fs.DMUTEX.o main.DMUTEX.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs-mutex lib/bst.o fs.DMUTEX.o main.DMUTEX.o

tecnicofs-rwlock: lib/bst.o fs.DRWLOCK.o main.DRWLOCK.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs-rwlock lib/bst.o fs.DRWLOCK.o main.DRWLOCK.o

lib/bst.o: lib/bst.c lib/bst.h
	$(CC) $(CFLAGS) -o lib/bst.o -c lib/bst.c

fs.nosync.o: fs.c fs.h lib/bst.h
	$(CC) $(CFLAGS) -o fs.nosync.o -c fs.c

fs.DMUTEX.o: fs.c fs.h lib/bst.h
	$(CC) $(CFLAGS) -DMUTEX -o fs.DMUTEX.o -c fs.c

fs.DRWLOCK.o: fs.c fs.h lib/bst.h
	$(CC) $(CFLAGS) -DRWLOCK -o fs.DRWLOCK.o -c fs.c

main.nosync.o: main.c fs.h lib/bst.h
	$(CC) $(CFLAGS) -o main.nosync.o -c main.c

main.DMUTEX.o: main.c fs.h lib/bst.h
	$(CC) $(CFLAGS) -DMUTEX -o main.DMUTEX.o -c main.c

main.DRWLOCK.o: main.c fs.h lib/bst.h
	$(CC) $(CFLAGS) -DRWLOCK -o main.DRWLOCK.o -c main.c

clean:
	@echo Cleaning...
	rm -f lib/*.o *.o tecnicofs-nosync tecnicofs-mutex tecnicofs-rwlock
	

run: tecnicofs
	./tecnicofs
