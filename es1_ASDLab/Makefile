CC = cc
CFLAGS ?= -std=c18 -O2 -g -Wall -Wextra -Wpedantic -Wconversion
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?=
LIBS = hybridsort record_io_bin
TEST_LIBS = hybridsort
MAIN_OBJS = build/main.o $(addprefix build/,$(addsuffix .o,$(LIBS)))
TEST_OBJS = build/test/hybridsort_test.o build/test/unity.o $(addprefix build/test/,$(addsuffix .o,$(TEST_LIBS)))

.PHONY: all test clean docs
all: bin/main_ex1 bin/test_ex1

test: bin/test_ex1
	./bin/test_ex1

build/%.o: src/%.c $(wildcard src/*.h) Makefile
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/test/%.o: src/%.c $(wildcard src/*.h) Makefile
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -DTESTING -c $< -o $@

bin/main_ex1: $(MAIN_OBJS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(CFLAGS) $^ $(LDLIBS) -o $@

bin/test_ex1: $(TEST_OBJS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(CFLAGS) $^ $(LDLIBS) -o $@

docs:
	doxygen Doxyfile

clean:
	rm -rf build bin
