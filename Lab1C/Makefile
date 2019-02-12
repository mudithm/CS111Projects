#NAME: Mudith Mallajosyula
#EMAIL: mudithm@g.ucla.edu
#ID: 404937201

CC=gcc
CFLAGS=-Wall -Wextra
TARFILES=lab1c.c Makefile README report.pdf

lab1c: lab1c.c
	$(CC)  -o simpsh lab1c.c $(CFLAGS)

clean:
	rm -f simpsh *.o *.tar.gz

check: badInputCheck badOutputCheck segFaultCheck catchCheck badArgumentCheck dumpcoreCheck
	
dist: $(TARFILES) 
	tar cvzf lab1-404937201.tar.gz $(TARFILES) 


badInputCheck: lab1c
	echo "test input" > fileIn; \
	chmod 000 fileIn; \
	./simpsh --rdonly fileIn; \
	if [[ $$? -ne 1 ]]; then \
		echo ">>>>>Failed Input Check"; \
	else \
		echo ">>>>>Passed Input Check"; \
	fi

	chmod 777 fileIn;
	rm fileIn 


badOutputCheck: lab1c
	echo "test output" > fileIn; \
	touch fileOut; \
	chmod 000 fileOut; \
	./simpsh --rdonly fileIn --wronly fileOut; \
	if [[ $$? -ne 1 ]]; then \
		echo ">>>>>Failed Output Check"; \
	else \
		echo ">>>>>Passed Output Check"; \
	fi

	chmod 777 fileOut;
	rm fileIn fileOut

segFaultCheck: lab1c
	./simpsh --abort; \
	if [[ $$? -ne 139 ]]; then \
		echo ">>>>>Failed segfault check"; \
	else \
		echo ">>>>>Passed segfault check"; \
	fi

catchCheck: lab1c
	./simpsh --abort --catch 11; \
	if [[ $$? -ne 139 ]]; then \
		echo ">>>>>Failed catch check"; \
	else \
		echo ">>>>>Passed catch check"; \
	fi

dumpcoreCheck: lab1c
	./simpsh --abort --catch 11 --default 11 --abort; \
	if [[ $$? -ne 139 ]]; then \
		echo ">>>>>Failed dump-core check"; \
	else \
		echo ">>>>>Passed dump-core check"; \
	fi



badArgumentCheck: lab1c
	./simpsh --bad; \
	if [[ $$? -ne 1 ]]; then \
		echo ">>>>>Failed bad argument check"; \
	else \
		echo ">>>>>Passed bad argument check"; \
	fi

