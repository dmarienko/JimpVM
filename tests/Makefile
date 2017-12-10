#
#  _           _    Common makefile for jimp java classes 
#   | . |   | |_|   * 24/01/2002: started
#  \  | | | | |     * 23-mar-2006: modified
#
# $Id: Makefile,v 1.5 2006/03/23 14:48:21 jimpos Exp $

JCC := javac
OPTIONS := -bootclasspath . -classpath . -source 1.4 -target 1.4

# Some smart definitions
SOURCES := $(shell echo *.java)
TESTS   := $(addsuffix .test,$(basename ${SOURCES}))
CLASSES := $(addsuffix .class,$(basename ${SOURCES}))

%.class : %.java
	$(JCC) $(OPTIONS) $<
	
%.test	: %.class
	@echo -e "\n ------- Start testing $(addsuffix .java,$(basename $<)) ------- \n"
	../vm $(basename $<)

all: $(CLASSES)

test: all $(TESTS)
	@echo -e "\n   === All tests was passed ! ==="
	@echo -e "   === Congratulations !!! === \n"

clean:
	rm -f *.class *~