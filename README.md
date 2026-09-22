# University Algorithms and Data Structures in C

Four laboratory projects developed by Simone Basso for the Algorithms and Data Structures course at the University of Turin. They explore generic sorting, hash tables, indexed priority queues, and sparse graphs through standalone C programs and automated tests.

## Projects

| Exercise | Implementation | Application |
| --- | --- | --- |
| [1 — Hybrid sorting](es1_ASDLab/README.md) | Quicksort with selection sort below a configurable threshold | Sort fixed-size binary records |
| [2 — Hash table](es2_ASDLab/README.md) | Separate chaining and automatic resizing | Find the most frequent word in a text |
| [3 — Priority queue](es3_ASDLab/README.md) | Ternary max-heap with a hash-based position index | Simulate non-preemptive priority scheduling |
| [4 — Graphs and BFS](es4_ASDLab/README.md) | Adjacency lists backed by a hash table | Traverse a network of places |

The exercise directories remain independent so each library can be built and studied separately. Shared implementations are copied locally to preserve that layout.

## Build and test

Requirements: a C18 compiler (GCC or Clang), Make, and Python 3 for the CLI regression tests. Doxygen is optional.

From the repository root:

```sh
make             # Build all applications and Unity test executables.
make test        # Run the C unit tests.
make check       # Build, run unit tests, and run CLI regression checks.
make clean       # Remove object files and executables.
```

Each exercise also supports `make`, `make test`, `make clean`, and `make docs` from its own directory. Output directories are created automatically.

To check memory access and undefined behavior with Clang:

```sh
make clean
make check CC=clang CFLAGS='-std=c18 -O1 -g -Wall -Wextra -Wpedantic -Wconversion -fsanitize=address,undefined -fno-omit-frame-pointer'
```

Clean before switching compiler options: Make does not track changes to command-line flags.

## Try the applications

Small, self-contained text and CSV inputs are included in `examples/`:

```sh
./es2_ASDLab/bin/main_ex2 examples/words.txt 4
./es3_ASDLab/bin/main_ex3 examples/tasks.csv /tmp/schedule.csv
./es4_ASDLab/bin/main_ex4 examples/places.csv a /tmp/bfs.txt
```

Exercise 1 requires binary records; its README explains the format and includes a small fixture generator. The automated tests create their own temporary binary inputs.

## Algorithms and complexity

| Operation | Expected time | Notes |
| --- | --- | --- |
| Hybrid sorting | O(n log n), for a fixed small threshold | O(n²) worst case; selection sort is quadratic |
| Hash lookup / insertion / removal | O(1), amortized for insertion | O(n) worst case with collisions |
| Queue top / size | O(1) | Ternary max-heap |
| Queue contains | O(1) expected | Hash lookup |
| Queue push / pop / removal | O(log n) expected, amortized for push | Includes hash index maintenance |
| Breadth-first traversal | O(V + E) expected | Requires well-distributed hashing |

Graph edge lookup and insertion scan an adjacency list and depend on vertex degree. Building a graph is therefore not guaranteed to take O(V + E) time. All complexity statements assume constant-time key comparison and hashing; strings add a dependency on key length.

## API conventions

Generic containers store caller-owned pointers. Keys and values must remain valid for as long as they are stored. Equal keys must produce the same hash. Do not mutate fields used by comparison or hashing while an element is in a container.

The priority queue uses the same comparison callback for ordering and equality. Include a unique tie-breaker when distinct elements have equal priorities. The graph comparison callback must define a total order; undirected edge enumeration uses that order to return each logical edge once.

Headers document ownership and return values. The hash-table insertion API returns `void`, as in the laboratory interface; callers that need to detect allocation failure must verify insertion. These educational libraries are not thread-safe.

## Data and generated files

Large course datasets, build artifacts, generated HTML documentation, and application outputs are excluded by `.gitignore`. The supplied local datasets are not required to build or run the tests. In particular, `records.bin` is a large local benchmark input and should not be committed.

Original Italian course instructions and reports are preserved locally in `.local-originals/`, which is also ignored. The public documentation describes the maintained code rather than exam administration. Historical benchmark results are not presented as measurements of this revised version.

## Development notes

ChatGPT was used as a development aid during the original project, as recorded in the original exercise reports. This revision includes code review, English documentation, error-handling improvements, and regression tests. Automated checks cover representative cases, not a proof of correctness for every input or allocation-failure path.

The bundled Unity sources retain their original copyright and licensing notices. No license is assigned here to the original project code.
