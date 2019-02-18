#NAME: Mudith Mallajosyula
#EMAIL: mudithm@g.ucla.edu
#ID: 404937201
SHELL:=/bin/bash
CC=gcc
CFLAGS=-Wall -Wextra -pthread -g -O0
TARFILES=lab2_add.c SortedList.h SortedList.c lab2_list.c Makefile lab2_add.csv lab2_list.csv README lab2_add.gp lab2_list.gp add_tests.sh list_tests.sh
GRAPHS=lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png

default: build

build: 
	$(CC) lab2_add.c $(CFLAGS) -o lab2_add
	$(CC) lab2_list.c SortedList.c $(CFLAGS) -o lab2_list

graphs: tests	
	gnuplot lab2_add.gp
	gnuplot lab2_list.gp

dist: graphs $(TARFILES)
	 tar -cvzf lab2a-404937201.tar.gz $(TARFILES) $(GRAPHS)

clean:
	rm -f lab2_add lab2_list *.tar.gz

tests: clean build add_tests.sh list_tests.sh 
	bash -c ./add_tests.sh
	bash -c ./list_tests.sh
