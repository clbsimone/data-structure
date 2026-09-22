CC = cc
CFLAGS ?= -std=c18 -O2 -g -Wall -Wextra -Wpedantic -Wconversion
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?=
LIBS = hash_table
TEST_LIBS = hash_table
MAIN_OBJS = build/main.o $(addprefix build/,$(addsuffix .o,$(LIBS)))
TEST_OBJS = build/test/hash_table_test.o build/test/unity.o $(addprefix build/test/,$(addsuffix .o,$(TEST_LIBS)))

.PHONY: all test clean docs
all: bin/main_ex2 bin/test_ex2

test: bin/test_ex2
	./bin/test_ex2

build/%.o: src/%.c $(wildcard src/*.h) Makefile
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/test/%.o: src/%.c $(wildcard src/*.h) Makefile
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -DTESTING -c $< -o $@

bin/main_ex2: $(MAIN_OBJS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(CFLAGS) $^ $(LDLIBS) -o $@

bin/test_ex2: $(TEST_OBJS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(CFLAGS) $^ $(LDLIBS) -o $@

docs:
	doxygen Doxyfile

clean:
	rm -rf build bin
