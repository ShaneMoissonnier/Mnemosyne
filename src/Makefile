.SUFFIXES:

CC=gcc

# uncomment to compile in 32bits mode (require gcc-*-multilib packages
# on Debian/Ubuntu)
#HOST32= -m32

CFLAGS= $(HOST32) -Wall -Werror -std=c99 -g -DMEMORY_SIZE=128000
CFLAGS+= -DDEBUG
LDFLAGS= $(HOST32)
INCLUDE=.

TESTS+=test_init test_base test_base_bzero test_cheese test_fusion test_frag test_fusion_bzero test_cheese_bzero test_unit
PROGRAMS=memshell

.PHONY: clean all test test_ls

all: $(PROGRAMS) $(TESTS)

test: $(TESTS)
	for file in $(TESTS);do ./$$file; done

# dépendances des binaires
DEPS=mem.o common.o

%:  $(DEPS) %.o 
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS)

TESTDIR=cutest

CuTest.o: cutest/CuTest.c cutest/CuTest.h
	$(CC) -c $< $(CFLAGS)

AllTests.o: cutest/AllTests.c cutest/AllTests.h cutest/CuTest.h
	$(CC) -c $< $(CFLAGS)

test_unit: $(DEPS) AllTests.o CuTest.o
	$(CC) -o $@ $^ $(CFLAGS)

# test avec des programmes existant
# création d'une librairie partagée
libmalloc.so: malloc_stub.o mem.o
	$(CC) -shared -Wl,-soname,$@ $^ -o $@

#test avec ls
test_ls: libmalloc.so
	LD_PRELOAD=./libmalloc.so ls	

# nettoyage
clean:
	rm -f *.o $(PROGRAMS) $(TESTS) libmalloc.so
