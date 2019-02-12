#NAME: Mudith Mallajosyula
#EMAIL: mudithm@g.ucla.edu
#ID: 404937201

CC=gcc
CFLAGS=-Wall -Wextra
TARFILES=lab1b.c Makefile README

lab1a: lab1b.c
	$(CC)  -o simpsh lab1b.c $(CFLAGS)

clean:
	rm -f simpsh *.o *.tar.gz

check:
	
dist: $(TARFILES) 
	tar cvzf lab1-404937201.tar.gz $(TARFILES) 


