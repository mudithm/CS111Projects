#NAME: Mudith Mallajosyula
#EMAIL: mudithm@g.ucla.edu
#ID: ---------

CC=gcc
CFLAGS=-Wall -Wextra
TARFILES=lab1a.c Makefile README

lab1a: lab1a.c
	$(CC)  -o simpsh lab1a.c $(CFLAGS)

clean:
	rm -f simpsh *.o *.tar.gz

check:
	
dist: $(TARFILES) 
	tar cvzf lab1-404937201.tar.gz $(TARFILES) 


