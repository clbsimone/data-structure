CC = cc
CFLAGS ?= -std=c18 -O2 -g -Wall -Wextra -Wpedantic -Wconversion
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?=
LIBS = priority_queue hash_table hybridsort
TEST_LIBS = priority_queue hash_table hybridsort
MAIN_OBJS = build/main.o $(addprefix build/,$(addsuffix .o,$(LIBS)))
TEST_OBJS = build/test/priority_queue_test.o build/test/unity.o $(addprefix build/test/,$(addsuffix .o,$(TEST_LIBS)))

.PHONY: all test clean docs
all: bin/main_ex3 bin/test_ex3

test: bin/test_ex3
	./bin/test_ex3

build/%.o: src/%.c $(wildcard src/*.h) Makefile
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/test/%.o: src/%.c $(wildcard src/*.h) Makefile
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -DTESTING -c $< -o $@

bin/main_ex3: $(MAIN_OBJS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(CFLAGS) $^ $(LDLIBS) -o $@

bin/test_ex3: $(TEST_OBJS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(CFLAGS) $^ $(LDLIBS) -o $@

docs:
	doxygen Doxyfile

clean:
	rm -rf build bin
