#NAME: Mudith Mallajosyula
#EMAIL: mudithm@g.ucla.edu
#ID: 404937201
SHELL:=/bin/bash
CC=gcc
CFLAGS=-Wall -Wextra -pthread -g -O0
TARFILES=lab2_add.c SortedList.h SortedList.c lab2_list.c Makefile lab2_add.csv lab2_list.csv README
GRAPHS=lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png

default: build

build: 
	$(CC) lab2_add.c $(CFLAGS) -o lab2_add
	$(CC) lab2_list.c SortedList.c $(CFLAGS) -o lab2_list

graphs: tests
	chmod 755 lab2_add.gp		
	chmod 755 lab2_list.gp
	./lab2_add.gp
	./lab2_list.gp

dist: graphs $(TARFILES)
	 tar -cvzf lab2a-404937201.tar.gz $(TARFILES) $(GRAPHS)

clean:
	rm -f lab2_add lab2_list *.png *.tar.gz *.csv

tests: clean build
	chmod 755 add_tests.sh		
	chmod 755 list_tests.sh		
	source add_tests.sh && ./add_tests.sh
	source list_tests.sh && ./list_tests.sh