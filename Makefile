CC = cc
CXX = g++
CFLAGS = -Wall -Wextra -O2 -fPIC -std=c11
CXXFLAGS = -Wall -Wextra -O2 -std=c++17 -I.
LDFLAGS = -pthread
TEST_LDFLAGS = -lgtest -lgtest_main -lpthread 
TEST_CFLAGS = $(CFLAGS) -DNOMAIN

LIB_NAME = libivm.so 
TEST_TARGET = run_tests

LIB_SRCS = IntMemoryManager.c ivm-api.c
MAIN_SRC = main.c
TEST_SRCS = tests/testmem.cpp tests/testapi.cpp tests/test1.cpp

LIB_OBJS = $(LIB_SRCS:.c=.o)
MAIN_OBJ = $(MAIN_SRC:.c=.o)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

PREFIX = /usr

all: $(LIB_NAME)

$(LIB_NAME): $(LIB_OBJS)
	$(CC) -shared -o $@ $(LIB_OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

main_test.o: main.c
	$(CC) $(TEST_CFLAGS) -c $< -o $@

test: $(TEST_OBJS) $(LIB_OBJS) main_test.o
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_OBJS) $(LIB_OBJS) main_test.o $(LDFLAGS) $(TEST_LDFLAGS)
	./$(TEST_TARGET)

install: $(LIB_NAME)
	install -d $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include/ivm
	install -d $(DESTDIR)$(PREFIX)/include/imm
	install -m 755 $(LIB_NAME) $(DESTDIR)$(PREFIX)/lib/
	install -m 644 ivm.h $(DESTDIR)$(PREFIX)/include/ivm/
	install -m 644 IntMemoryManager.h $(DESTDIR)$(PREFIX)/include/imm/
	sudo ldconfig
	@echo "Successfully installed IVM."

clean:
	rm -f *.o tests/*.o $(LIB_NAME) $(TEST_TARGET)

.PHONY: all clean install test