#NAME: Mudith Mallajosyula
#EMAIL: mudithm@g.ucla.edu
#ID: 404937201
SHELL:=/bin/bash
CC=gcc
CFLAGS=-Wall -Wextra
TARFILES=Makefile README lab3a.c ext2_fs.h

default: build

build: 
	$(CC) lab3a.c $(CFLAGS) -o lab3a


dist: $(TARFILES)
	tar -cvzf lab3a-404937201.tar.gz $(TARFILES)

clean:
	rm -f lab3a *.tar.gz
