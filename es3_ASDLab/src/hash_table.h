#ifndef HASH_TABLE_H_SIMONE_BASSO
#define HASH_TABLE_H_SIMONE_BASSO

#include <stddef.h>

/**
 * @brief Opaque structure representing a hash table.
 *
 * The internal structure is hidden from the user.
 * The table stores generic (void*) keys and values.
 */
typedef struct HashTable HashTable;

/**
 * @brief Create a new hash table.
 *
 * The hash table uses separate chaining to resolve collisions.
 *
 * @param compare   Function used to compare two keys.
 *                  Must return 0 if the keys are equal.
 * @param hash_func Function used to compute the hash of a key.
 *
 * @return Pointer to a new HashTable, or NULL on failure.
 */
HashTable *hash_table_create(int (*compare)(const void *, const void *),
                             unsigned long (*hash_func)(const void *));

/**
 * @brief Insert or update a (key, value) pair in the table.
 *
 * If the key is already present, its value is replaced; the original key pointer stays.
 * Keys and values are borrowed. NULL values are allowed; NULL keys are ignored.
 * Allocation failure leaves the mapping unchanged. Verify insertion if required.
 *
 * @param table Pointer to the hash table.
 * @param key   Pointer to the key.
 * @param value Pointer to the value.
 */
void hash_table_put(HashTable *table, const void *key, const void *value);

/**
 * @brief Retrieve the value associated with a key.
 *
 * @param table Pointer to the hash table.
 * @param key   Pointer to the key to search for.
 *
 * @return Associated value, or NULL for a missing key or a stored NULL value.
 * Use hash_table_contains_key to distinguish these cases.
 */
void *hash_table_get(const HashTable *table, const void *key);

/**
 * @brief Check if a key exists in the table.
 *
 * @param table Pointer to the hash table.
 * @param key   Pointer to the key.
 *
 * @return 1 if the key is present, 0 otherwise.
 */
int hash_table_contains_key(const HashTable *table, const void *key);

/**
 * @brief Remove a key (and its value) from the table.
 *
 * If the key is not present, nothing happens.
 *
 * @param table Pointer to the hash table.
 * @param key   Pointer to the key to remove.
 */
void hash_table_remove(HashTable *table, const void *key);

/**
 * @brief Return the number of elements stored in the table.
 *
 * @param table Pointer to the hash table.
 *
 * @return Number of elements.
 */
int hash_table_size(const HashTable *table);

/**
 * @brief Return an array with all keys in the table.
 *
 * The returned array must be freed by the caller with free().
 * The keys themselves are not copied.
 *
 * @param table Pointer to the hash table.
 *
 * @return Array of keys, or NULL if the table is empty or on failure.
 */
void **hash_table_keyset(const HashTable *table);

/**
 * @brief Free the entire hash table.
 *
 * This function frees internal structures but does NOT free
 * the keys and values stored in the table.
 *
 * @param table Pointer to the hash table to destroy.
 */
void hash_table_free(HashTable *table);

#endif
