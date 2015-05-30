CPPC=g++ -std=c++11
CC=gcc

PROG=yapeg

INCLUDES=-I.
LD_FLAGS=

IS_GTEST=yes
GTEST_DIR=../gtest

ifeq (,$(IS_GTEST))
	TASK=$(PROG).tsk
	SRCS=$(filter-out $(wildcard *.t.cpp), $(wildcard *.cpp)) $(wildcard *.c)
else
	TASK=$(PROG)_gtest.tsk
	INCLUDES+=-I$(GTEST_DIR)/include
	LD_FLAGS+=-L$(GTEST_DIR)/build -lgtest -lgtest_main
	SRCS=$(filter-out $(wildcard *.m.cpp), $(wildcard *.cpp)) $(wildcard *.c)
endif

OBJS=$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(SRCS)))

build: $(TASK)

$(TASK): $(OBJS)
	$(CPPC) $(LD_FLAGS) $(OBJS) -o $@

.cpp.o:
	$(CPPC) -c -Wall $(INCLUDES) $< -o $@

.c.o:
	$(CC) -c $(INCLUDES) $< -o $@

clean:
	\rm -f *.o *.tsk
