UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
    CC=clang++
    CC+=-D_XOPEN_SOURCE -Wno-deprecated-declarations
    LIBCPU=libcpu_macos.o
else
    CC=g++
    LIBCPU=libcpu.o
endif

CC+=-g -Wall -std=c++17

# List of source files for your thread library
THREAD_SOURCES=cpu.cpp thread.cpp tcb.cpp cv.cpp mutex.cpp raii.cpp

# Generate the names of the thread library's object files
THREAD_OBJS=${THREAD_SOURCES:.cpp=.o}

all: libthread.o test_spec test_yield test_lab test_ping_pong test_join test_deadlock test_broadcast test_illegal

# Compile the thread library and tag this compilation
libthread.o: ${THREAD_OBJS}
	./autotag.sh push
	ld -r -o $@ ${THREAD_OBJS}

test_spec: test_spec.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread
# Compile an application program
test_yield: test_yield.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

test_lab: test_lab.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

test_ping_pong: test_ping_pong.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

test_join: test_join.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

test_deadlock: test_deadlock.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread
 
test_broadcast: test_broadcast.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

test_illegal: test_illegal_unlock.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

test_questionable: test_questionable.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread
# test_pizza: test_pizza.cpp libthread.o libthread2.o pizza.cpp utils.cpp ${LIBCPU}
# 	${CC} -o $@ test_pizza.cpp \
# 	    -Wl,-flat_namespace \
# 	    -Wl,-multiply_defined,suppress \
# 	    -Wl,-force_load,libthread2.o \
# 	    libthread.o pizza.cpp utils.cpp ${LIBCPU} -ldl -pthread

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${THREAD_OBJS} libthread.o test_spec test_yield test_lab test_ping_pong test_join test_deadlock test_illegal test_broadcast test_questionable
  