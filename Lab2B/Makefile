#NAME: Mudith Mallajosyula
#EMAIL: mudithm@g.ucla.edu
#ID: 404937201
SHELL:=/bin/bash
CC=gcc
CFLAGS=-Wall -Wextra -pthread -g
TARFILES=lab2_add.c SortedList.h SortedList.c lab2_list.c Makefile lab2_add.csv lab2_list.csv README lab2_add.gp lab2_list.gp add_tests.sh list_tests.sh
GRAPHS=lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png

default: build

build: 
	$(CC) lab2_add.c $(CFLAGS) -o lab2b_add
	$(CC) lab2_list.c SortedList.c $(CFLAGS) -o lab2b_list

graphs: tests	
	gnuplot lab2b.gpo

dist: graphs $(TARFILES)
	 tar -cvzf lab2a-404937201.tar.gz $(TARFILES) $(GRAPHS)

profile: build
	mv lab2b_list lab2b_temp_list
	$(CC) lab2_list.c SortedList.c $(CFLAGS) -L /usr/local/cs/gperftools-2.7/lib -lprofiler -o lab2b_list
	# do some stuff here
	# remove old gperf if it exists
	rm -f raw.gperf
	LD_PRELOAD=/usr/local/cs/gperftools-2.7/lib/libprofiler.so \
	CPUPROFILE=raw.gperf ./lab2b_list --threads=12 --iterations=1000 --sync=s
	pprof --text lab2b_list raw.gperf > profile.out
	pprof --list=listAdd_spin lab2b_list raw.gperf >> profile.out
	rm -f raw.gperf
	mv lab2b_temp_list lab2b_list

clean:
	rm -f lab2b_add lab2b_list *.tar.gz

tests: clean build add_tests.sh list_tests.sh 
#	bash -c ./add_tests.sh
#	bash -c ./list_tests.sh
	chmod +x list_test.sh
	bash -c ./list_test.sh
