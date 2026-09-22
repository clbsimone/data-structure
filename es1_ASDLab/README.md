# Exercise 1 — Hybrid Sorting

A generic, in-place hybrid of quicksort and selection sort, applied to binary records.

## Algorithm

`hybridsort(base, nitems, size, k, compar)` uses median-of-three pivot selection and Hoare partitioning. Subarrays with **fewer than** `k` elements use selection sort. A threshold of 0 or 1 disables that switch. The smaller partition is processed recursively and the larger one iteratively, keeping recursion depth O(log n). The sort is not stable.

For a fixed small threshold, expected time is O(n log n); worst-case time is O(n²). A large threshold can make selection sort dominate. A pivot buffer requires O(size) extra space; allocation failure falls back to selection sort.

## Build and usage

```sh
make
make test
./bin/main_ex1 input.bin output.bin 1 10
```

Arguments: input path, output path, sorting field (1–4), and nonnegative threshold `k`.

| Field argument | Record member | Type |
| --- | --- | --- |
| 1 | `id` | Unsigned 64-bit integer |
| 2 | `field1` | 32-bit floating-point number |
| 3 | `field2` | Signed 64-bit integer |
| 4 | `field3` | 16-byte string buffer |

Records occupy exactly 36 bytes on disk, without structure padding: 8 bytes for `id`, 4 for `field1`, 8 for `field2`, and 16 for `field3`. This implementation requires a little-endian host with 32-bit IEEE 754 floats. Strings have at most 15 characters plus a terminator. Floating-point NaNs sort after ordinary numbers.

Create a small input from this directory:

```sh
python3 - <<'PY'
import struct
with open('sample.bin', 'wb') as output:
    for record in [(3, 1.5, -4, b'gamma'), (1, 2.0, 9, b'alpha'), (2, -1.0, 0, b'beta')]:
        output.write(struct.pack('<Qfq16s', *record))
PY
./bin/main_ex1 sample.bin sorted.bin 1 10
```

The application loads all records into memory before sorting. Keep input and output files distinct, including symbolic links and hard links. Empty input is accepted; truncated records and invalid numeric arguments are rejected. Output errors return failure.

## Tests and experiments

Unity tests cover helpers, primitive types, duplicates, sorted and reverse-sorted arrays, empty arrays, and threshold choices. The root `make check` additionally compares binary output against independently sorted data for every field and several thresholds.

The original report explored thresholds from 0 to 1,000,000 on the course dataset. Those historical timings are retained in the local backup; rerun benchmarks to measure the current implementation on your hardware.
