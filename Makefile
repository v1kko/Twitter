CC=gcc

WARNING_FLAGS=-Wall -Wextra -Werror-implicit-function-declaration -Wshadow -Wstrict-prototypes -pedantic-errors
CFLAGS=-g -D_XOPEN_SOURCE -D_POSIX_SOURCE -std=c99 $(WARNING_FLAGS)
LDFLAGS=-g 

.c.o:
	$(CC) -c $(CFLAGS) $<

all: daemon client 

daemon: daemon.o daemoninit.o daemonclient.o
	$(CC) $(LDFLAGS) -o twitterd daemon.o daemoninit.o daemonclient.o

client: client.o
	$(CC) $(LDFLAGS) -o client client.o

clean:
	rm -f *.o
	rm -f client
	rm -f twitterd

verslag:
	pdflatex verslag.tex
	rm -f *.log
	rm -f *.aux

daemonclient.o: daemon.h daemonclient.c queue.h
daemon.o: daemon.h daemon.c
daemoninit.o: daemon.h daemoninit.c
client.o: client.c
