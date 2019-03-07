#NAME: Mudith Mallajosyula
#EMAIL: mudithm@g.ucla.edu
#ID: 404937201

TARFILES=Makefile README lab3b.py

default: build

build: 
	cp lab3b.py lab3b
	chmod +x lab3b

dist: $(TARFILES)
	tar -cvzf lab3b-404937201.tar.gz $(TARFILES)

clean:
	rm -f lab3b *.tar.gz
