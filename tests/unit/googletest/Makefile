
CPPFLAGS=-I include -I .
CFLAGS=-Wall -Wextra
CC=clang
CXX=clang++

all : gtest.a gtest_main.a

gtest.a : src/gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : src/gtest-all.o src/gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

clean :
	rm -f src/*.o gtest.a gtest_main.a
