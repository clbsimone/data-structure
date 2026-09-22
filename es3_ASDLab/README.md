# Exercise 3 — Ternary Priority Queue and Scheduling

A generic max-priority queue backed by a ternary heap and a hash table mapping elements to heap positions. Heap swaps update the index, allowing arbitrary removal without scanning the heap.

## Build and usage

```sh
make
make test
./bin/main_ex3 ../examples/tasks.csv /tmp/schedule.csv
```

The input is a headerless CSV with four integer fields per row:

```text
id,start,length,priority
```

This line describes the format; do not include it as a header. Arrival times must be nonnegative and durations strictly positive. Values must fit a C `int`; task IDs should be unique. Malformed rows cause failure. The parser supports plain numeric CSV, without quoted fields, and uses a 256-byte line buffer.

Tasks are sorted by arrival time using hybrid quicksort/selection sort. This exercise's sorting variant samples five positions for its pivot, falling back to median-of-three on small ranges.

The scheduler is non-preemptive: a running task finishes before another starts. Among ready tasks it chooses higher priority first, then earlier arrival, then lower ID. Idle periods jump directly to the next arrival. Simulation timestamps use 64-bit integers.

Output is headerless CSV:

```text
1,0,3
2,3,4
3,4,6
4,20,21
```

Each row contains `id,start_time,end_time`.

## Queue contract and complexity

The queue borrows elements; it owns only its heap and index metadata. Elements must remain alive and unchanged while queued. Comparison defines both priority and equality, so distinct elements with equal priority need a tie-breaker. Equal elements must hash equally.

`push` and `remove` return 1 on success, 0 for an existing or missing element respectively, and -1 for invalid arguments or a failed push allocation. `top` returns NULL when empty; `pop` is a no-op when empty. See the header for all return conventions.

Top and size are O(1); membership is O(1) expected. Push is amortized expected O(log n), and pop/removal are expected O(log n), assuming well-distributed hashing. Scheduling is expected O(n log n), with an O(n²) sorting worst case. Storage is O(n).

## Tests

Unity tests exercise queue creation, duplicates, ordering, arbitrary removal, empty queues, and larger inputs. CLI tests verify tie-breaking, idle periods, malformed CSV rejection, and timestamps beyond the signed 32-bit range.
