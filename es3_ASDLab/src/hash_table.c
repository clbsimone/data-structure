#include "hash_table.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Node of the list used for chaining collisions */
typedef struct HashNode{
    void *key;
    void *value;
    struct HashNode *next; // pointer to the next node in the list
} HashNode;

/* Main hash table structure */
struct HashTable{
    HashNode **buckets;                         // array of lists (size = capacity)
    size_t capacity;                            // m: number of buckets
    size_t size;                                // N: number of stored (key, value) pairs
    int (*compare)(const void *, const void *); // key comparison function
    unsigned long (*hash_func)(const void *);   // hash function on keys
};

/* Standard parameters: initial capacity and max load factor */
#define INITIAL_CAPACITY 16
#define LOAD_FACTOR 0.75

/* SUPPORT FUNCTIONS (STATIC) */
static HashNode *hash_node_create(const void *key, const void *value);
static void free_buckets(HashNode **buckets, size_t capacity);
static int hash_table_resize(HashTable *table, size_t new_capacity);

HashTable *hash_table_create(int (*compare)(const void *, const void *), unsigned long (*hash_func)(const void *)){
    if (!compare || !hash_func)
        return NULL;

    HashTable *table = (HashTable *)malloc(sizeof(HashTable));
    if (!table)
        return NULL;

    table->capacity = INITIAL_CAPACITY;
    table->size = 0;
    table->compare = compare;
    table->hash_func = hash_func;

    // Allocate the buckets array and initialize all entries to NULL
    table->buckets = (HashNode **)calloc(table->capacity, sizeof(HashNode *));
    if (table->buckets == NULL){
        free(table);
        return NULL;
    }

    return table;
}

void hash_table_put(HashTable *table, const void *key, const void *value){
    if (!table || !key)
        return;

    unsigned long h = table->hash_func(key);
    size_t idx = h % table->capacity;

    HashNode *curr = table->buckets[idx];

    // Search the key in the list, if it already exists
    while (curr != NULL){
        if (table->compare(curr->key, key) == 0){
            // key found -> update the value
            curr->value = (void *)value;
            return;
        }
        curr = curr->next;
    }

    if (table->size == INT_MAX) return;
    /* Updating existing keys never allocates or resizes the table. */
    if (table->size + 1 > table->capacity - table->capacity / 4) {
        if (table->capacity > SIZE_MAX / 2 ||
            !hash_table_resize(table, table->capacity * 2)) return;
        idx = h % table->capacity;
    }

    // key not found -> insert a new node at the head of the list
    HashNode *new_node = hash_node_create(key, value);
    if (new_node == NULL)
        return;

    new_node->next = table->buckets[idx];
    table->buckets[idx] = new_node;

    table->size++;
}

void *hash_table_get(const HashTable *table, const void *key){
    if (!table || !key)
        return NULL;

    unsigned long h = table->hash_func(key);
    size_t idx = h % table->capacity;

    HashNode *curr = table->buckets[idx];
    while (curr != NULL){
        if (table->compare(curr->key, key) == 0)
            return curr->value;
        curr = curr->next;
    }

    return NULL;
}

int hash_table_contains_key(const HashTable *table, const void *key){
    return (hash_table_get(table, key) != NULL);
}

void hash_table_remove(HashTable *table, const void *key){
    if (!table || !key)
        return;

    unsigned long h = table->hash_func(key);
    size_t idx = h % table->capacity;

    HashNode *curr = table->buckets[idx];
    HashNode *prev = NULL;

    while (curr != NULL){
        if (table->compare(curr->key, key) == 0){
            // found the node to remove
            if (prev == NULL)
                // node is at the head of the list
                table->buckets[idx] = curr->next;
            else
                prev->next = curr->next;

            free(curr); // does NOT free key or value (managed by the caller)
            table->size--;
            return;
        }

        prev = curr;
        curr = curr->next;
    }
}

int hash_table_size(const HashTable *table){
    if (table == NULL)
        return 0;

    return (int)table->size;
}

void **hash_table_keyset(const HashTable *table){
    if (table == NULL || table->size == 0)
        return NULL;

    void **keys = (void **)malloc(table->size * sizeof(void *));
    if (keys == NULL){
        return NULL;
    }

    size_t idx = 0;

    // scan all buckets and copy the key pointers
    for (size_t i = 0; i < table->capacity; i++){
        HashNode *curr = table->buckets[i];
        while (curr != NULL){
            keys[idx++] = curr->key;
            curr = curr->next;
        }
    }

    return keys;
}

void hash_table_free(HashTable *table){
    if (table == NULL)
        return;

    free_buckets(table->buckets, table->capacity);
    free(table);
}

static HashNode *hash_node_create(const void *key, const void *value){
    HashNode *node = (HashNode *)malloc(sizeof(HashNode));
    if (!node)
        return NULL;

    node->key = (void *)key;
    node->value = (void *)value;
    node->next = NULL;

    return node;
}

static void free_buckets(HashNode **buckets, size_t capacity){
    if (buckets == NULL)
        return;

    for (size_t i = 0; i < capacity; i++){
        HashNode *curr = buckets[i];

        while (curr != NULL){
            HashNode *tmp = curr;
            curr = curr->next;
            free(tmp); // key and value are NOT freed here
        }
    }

    free(buckets);
}

static int hash_table_resize(HashTable *table, size_t new_capacity){
    HashNode **new_buckets = (HashNode **)calloc(new_capacity, sizeof(HashNode *));
    if (!new_buckets)
        return 0;

    // reinsert all nodes into the new buckets
    for (size_t i = 0; i < table->capacity; i++){
        HashNode *curr = table->buckets[i];
        while (curr != NULL){
            HashNode *next = curr->next;

            unsigned long h = table->hash_func(curr->key);
            size_t idx = h % new_capacity;

            // standard "insert at head of list" pattern
            curr->next = new_buckets[idx];
            new_buckets[idx] = curr;

            curr = next;
        }
    }

    free(table->buckets);
    table->buckets = new_buckets;
    table->capacity = new_capacity;

    return 1;
}
