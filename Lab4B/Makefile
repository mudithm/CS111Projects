#NAME: Mudith Mallajosyula
#EMAIL: mudithm@g.ucla.edu
#ID: 404937201

TARFILES=Makefile README lab4b.c testscript.sh
CFLAGS=-lmraa -Wall -Wextra

default: build

build: lab4b.c
	gcc $(CFLAGS) -o lab4b lab4b.c -lm

dist: $(TARFILES)
	tar -cvzf lab4b-404937201.tar.gz $(TARFILES)

check: build
	chmod +x testscript.sh
	./testscript.sh

clean:
	rm -f lab4b *.tar.gz
