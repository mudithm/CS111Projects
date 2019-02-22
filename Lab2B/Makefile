#NAME: Mudith Mallajosyula
#EMAIL: mudithm@g.ucla.edu
#ID: 404937201
SHELL:=/bin/bash
CC=gcc
CFLAGS=-Wall -Wextra -pthread -g
TARFILES=SortedList.h SortedList.c lab2_list.c Makefile lab2b_list.csv README lab2b_list.gp list_test.sh profile.out
GRAPHS=lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png

default: build

build: 
	$(CC) lab2_list.c SortedList.c $(CFLAGS) -o lab2_list

graphs: tests
	chmod +x lab2b_list.gp
	gnuplot lab2b_list.gp

dist: graphs profile $(TARFILES)
	rm -f profile
	tar -cvzf lab2b-404937201.tar.gz $(TARFILES) $(GRAPHS)

profile: build
	mv lab2_list lab2b_temp_list
	$(CC) lab2_list.c SortedList.c $(CFLAGS) -L /usr/local/cs/gperftools-2.7/lib -lprofiler -o lab2_list
	# do some stuff here
	# remove old gperf if it exists
	rm -f raw.gperf
	LD_PRELOAD=/usr/local/cs/gperftools-2.7/lib/libprofiler.so \
	CPUPROFILE=raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text lab2_list raw.gperf > profile
	pprof --list=listAdd_spin lab2_list raw.gperf >> profile
	rm -f raw.gperf
	mv lab2b_temp_list lab2_list
	cp profile profile.out


clean:
	rm -f lab2_list *.tar.gz

tests: clean build list_test.sh 
#	bash -c ./add_tests.sh
#	bash -c ./list_tests.sh
	chmod +x list_test.sh
	bash -c ./list_test.sh
