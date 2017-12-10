#
#  _           _    Common makefile for jimp java classes 
#   | . |   | |_|   * 24/01/2002: started
#  \  | | | | |     * 23-mar-2006: modified
#
# $Id: common.mk,v 1.2 2006/03/23 14:32:11 jimpos Exp $

JCC := javac
OPTIONS := -bootclasspath ../../ -classpath ../../ -source 1.4 -target 1.4

# Some smart definitions
SOURCES := $(shell echo *.java)
CLASSES := $(addsuffix .class,$(basename ${SOURCES}))

%.class : %.java
	$(JCC) $(OPTIONS) $<

all: $(CLASSES)

clean:
	rm -f *.class *~