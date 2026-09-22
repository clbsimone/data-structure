CC = cc
CFLAGS ?= -std=c18 -O2 -g -Wall -Wextra -Wpedantic -Wconversion
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?=
LIBS = graph hash_table bfs
TEST_LIBS = graph hash_table bfs
MAIN_OBJS = build/main.o $(addprefix build/,$(addsuffix .o,$(LIBS)))
TEST_OBJS = build/test/test.o build/test/unity.o $(addprefix build/test/,$(addsuffix .o,$(TEST_LIBS)))

.PHONY: all test clean docs
all: bin/main_ex4 bin/test_ex4

test: bin/test_ex4
	./bin/test_ex4

build/%.o: src/%.c $(wildcard src/*.h) Makefile
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/test/%.o: src/%.c $(wildcard src/*.h) Makefile
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -DTESTING -c $< -o $@

bin/main_ex4: $(MAIN_OBJS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(CFLAGS) $^ $(LDLIBS) -o $@

bin/test_ex4: $(TEST_OBJS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(CFLAGS) $^ $(LDLIBS) -o $@

docs:
	doxygen Doxyfile

clean:
	rm -rf build bin
