#NAME: Mudith Mallajosyula
#EMAIL: mudithm@g.ucla.edu
#ID: ---------

CC=gcc
CFLAGS=-Wall -Wextra
TARFILES=lab0.c Makefile backtrace.png breakpoint.png README

lab0: lab0.c
	$(CC)  -o lab0 lab0.c $(CFLAGS)

clean:
	rm -f lab0 *.o *.tar.gz

check: badInputCheck badOutputCheck segFaultCheck catchCheck dumpcoreCheck badArgumentCheck

dist: $(TARFILES) 
	tar cvzf lab0----------.tar.gz $(TARFILES) 


badInputCheck: lab0
	echo "test input" > fileIn; \
	chmod 000 fileIn; \
	./lab0 --input=fileIn --output=test; \
	if [[ $$? -ne 2 ]]; then \
		echo ">>>>>Failed Input Check"; \
	else \
		echo ">>>>>Passed Input Check"; \
	fi

	chmod 777 fileIn;
	rm fileIn 


badOutputCheck: lab0
	echo "test output" > fileIn; \
	touch fileOut; \
	chmod 000 fileOut; \
	./lab0 --input=fileIn --output=fileOut; \
	if [[ $$? -ne 3 ]]; then \
		echo ">>>>>Failed Output Check"; \
	else \
		echo ">>>>>Passed Output Check"; \
	fi

	chmod 777 fileOut;
	rm fileIn fileOut

segFaultCheck: lab0
	./lab0 --segfault; \
	if [[ $$? -ne 139 ]]; then \
		echo ">>>>>Failed segfault check"; \
	else \
		echo ">>>>>Passed segfault check"; \
	fi

catchCheck: lab0
	./lab0 --segfault --catch; \
	if [[ $$? -ne 4 ]]; then \
		echo ">>>>>Failed catch check"; \
	else \
		echo ">>>>>Passed catch check"; \
	fi

dumpcoreCheck: lab0
	./lab0 --segfault --catch --dump-core; \
	if [[ $$? -ne 139 ]]; then \
		echo ">>>>>Failed dump-core check"; \
	else \
		echo ">>>>>Passed dump-core check"; \
	fi



badArgumentCheck: lab0
	./lab0 --bad; \
	if [[ $$? -ne 1 ]]; then \
		echo ">>>>>Failed bad argument check"; \
	else \
		echo ">>>>>Passed bad argument check"; \
	fi

