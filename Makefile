# paths
CC  = gcc
CXX = gcc
LD  = gcc

#JIMP_INCLUDE_PATH=../include

CFLAGS   := -I. -g2 -O2 -Wall
LDFLAGS  := #-L$(JIMP_LIB_PATH)

# Make rules
%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

%.o:%.cc
	$(CC) -c $(CXXFLAGS) $< -o $@

%.o:%.cpp
	$(CC) -c $(CXXFLAGS) $< -o $@

#%:%.o
#	$(LD) $^ $(LDFLAGS) -o $@

NSRC := $(shell echo native/*.c)
NOBJS := $(addsuffix .o,$(basename ${NSRC}))

OBJS = J_Class.o J_Misc.o J_Memory.o J_Native.o J_Machine.o J_Arrays.o J_Gc.o ${NOBJS}
HDRS = $(wildcard *.h)

# Debug items:
#  DEBUG_CONSTS - output class constants loading
#  DEBUG_JVM    - output jvm executing trace
DEBUG= -DDEBUG_JVM -DDEBUG_CONSTS -DDISASSEMBLER
CFLAGS  += $(DEBUG)

NAME    = vm

$(NAME): $(OBJS) classes

$(OBJS): $(HDRS) 

$(NAME): $(OBJS)
	$(LD) -o $(NAME) $(OBJS) $(LDFLAGS)

classes:
	make -C java

install: $(NAME)
	@echo "Installing $(NAME)"

natives.h:
	make -C native

J_Native.c: natives.h
	
clean:	
	@echo "Cleaning...$(NAME)"
	@rm -f *.o *~ *.b a.out $(NAME)
	make clean -C native
	make clean -C java
	make clean -C tests

all: clean install

test: install
	make test -C tests

.PHONY: clean all install
.IGNORE: clean

