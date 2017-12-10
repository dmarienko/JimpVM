##
# Makefile for native functions 
#
GENERATOR = ./nfgenerate.pl

ALL: natives.h

natives.h:
	$(GENERATOR) ./ > ../natives.h

clean:	
	@echo "Cleaning..."
	@rm -f *.o *~ *.b a.out ../natives.h

all: clean install

.PHONY: clean all install
.IGNORE: clean
