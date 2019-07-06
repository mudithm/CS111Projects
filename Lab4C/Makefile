#NAME: Mudith Mallajosyula
#EMAIL: mudithm@g.ucla.edu
#ID: 404937201

TARFILES=Makefile README lab4c_tcp.c lab4c_tls.c 
CFLAGS=-lmraa -Wall -Wextra -D_POSIX_C_SOURCE=201112L

default: build

build: lab4c_tcp.c
	gcc $(CFLAGS) -o lab4c_tcp lab4c_tcp.c -lm
	gcc $(CFLAGS) -o lab4c_tls lab4c_tls.c -lm -lssl -lcrypto

dist: $(TARFILES)
	tar -cvzf lab4c-404937201.tar.gz $(TARFILES)


clean:
	rm -f lab4c_tcp *.tar.gz
