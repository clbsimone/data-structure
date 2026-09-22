# Exercise 2 — Hash Table and Word Frequencies

A generic hash table using separate chaining. The table starts with 16 buckets and doubles before inserting a new key above a 0.75 load factor. Updating an existing key does not resize the table.

## Build and usage

```sh
make
make test
./bin/main_ex2 ../examples/words.txt 4
```

Arguments: text file and minimum word length, a nonnegative integer. The application prints the most frequent qualifying word and its count. Ties are resolved lexicographically. If no word qualifies, it prints nothing.

Words are sequences recognized by C `isalpha`, normalized with `tolower`. In the default C locale this is ASCII-oriented, not Unicode tokenization. Punctuation and digits separate words. The current fixed word buffer retains at most 1,023 characters per token; longer tokens are truncated.

## API and ownership

`hash_table_create` takes comparison and hash callbacks. Equal keys must have equal hashes. `put` inserts or updates, `get` returns a value, `contains_key` checks membership, and `remove` deletes a mapping. A stored value may be `NULL`; membership is independent of the value.

Keys and values are borrowed. Removing entries or freeing the table does not free caller-owned data. `hash_table_keyset` returns a newly allocated array of borrowed key pointers, which the caller must free. Its order is unspecified. `hash_table_put` has no return value; verify the mapping when insertion success matters.

Expected lookup, insertion, and removal are O(1), amortized for insertion; collision-heavy worst cases are O(n). Resizing and key enumeration take O(n + capacity), with O(n + capacity) storage overall.

## Tests

Unity tests cover creation, updates, membership (including NULL values), removal, key enumeration, resizing, and string keys. Root-level CLI regression checks cover punctuation, case normalization, frequency ties, and invalid arguments.
