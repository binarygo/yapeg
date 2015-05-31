CPPC=g++ -std=c++11
CC=gcc

PROG=yapeg

INCLUDES=-I.
LD_FLAGS=

get_objs = $(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(1)))

# gtest
GTEST_TARGET=$(PROG)_gtest.tsk
GTEST_DIR=../gtest
INCLUDES+=-I$(GTEST_DIR)/include
LD_FLAGS+=-L$(GTEST_DIR)/build -lgtest -lgtest_main
GTEST_SRCS=$(filter-out $(wildcard *.m.cpp), $(wildcard *.cpp)) $(wildcard *.c)
GTEST_OBJS=$(call get_objs,$(GTEST_SRCS))

# lib
LIB_TARGET=lib$(PROG).a
LIB_SRCS=$(filter-out $(wildcard *.t.cpp), $(wildcard *.cpp)) $(wildcard *.c)
LIB_OBJS=$(call get_objs,$(LIB_SRCS))

.PHONY: gtest_build
gtest_build: $(GTEST_TARGET)

$(GTEST_TARGET): $(GTEST_OBJS)
	$(CPPC) $(LD_FLAGS) $^ -o $@

.PHONY: lib_build
lib_build: $(LIB_TARGET)

$(LIB_TARGET): $(LIB_OBJS)
	ar rcs $@ $^

.cpp.o:
	$(CPPC) -c -Wall $(INCLUDES) $< -o $@

.c.o:
	$(CC) -c $(INCLUDES) $< -o $@

.PHONY: clean
clean:
	\rm -f *.o *.tsk *.a
