#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include "priority_queue.h"
#include "hash_table.h"

struct PriorityQueue{
    void **heap;  // dynamic array that stores heap elements
    int size;     // current number of elements in the heap
    int capacity; // allocated size of the heap array

    int (*compare)(const void *, const void *); // comparison function for priorities
    unsigned long (*hash_f)(const void *);      // hash function for elements (keys in the hash table)

    HashTable *index_table; // maps element -> pointer to its index in the heap array
};

// MACROS FOR TERNARY HEAP (0-based indexing)
#define PARENT(i) (((i) - 1) / 3)
#define CHILD(i, k) (3 * (i) + 1 + (k)) // k = 0,1,2
#define K 3                             // number of children (ternary heap)

// SUPPORT FUNCTIONS
static int ensure_capacity(PriorityQueue *pq);
static void swap_position(PriorityQueue *pq, int i, int j);
static void shift_up(PriorityQueue *pq, int pos);
static void shift_down(PriorityQueue *pq, int pos);

PriorityQueue *priority_queue_create(int (*compare)(const void *, const void *), unsigned long (*hf)(const void *)){
    if (!compare || !hf)
        return NULL;

    PriorityQueue *pq = malloc(sizeof(PriorityQueue));
    if (!pq)
        return NULL;

    pq->heap = NULL;
    pq->size = 0;
    pq->capacity = 0;

    pq->compare = compare;
    pq->hash_f = hf;

    // the hash table maps: element -> (size_t*) index_in_heap
    pq->index_table = hash_table_create(compare, hf);
    if (!pq->index_table){
        free(pq);
        return NULL;
    }

    return pq;
}

int priority_queue_push(PriorityQueue *pq, void *elem){
    if (!pq || !elem)
        return -1;

    // do not insert duplicates (based on compare/hash in the hash table)
    if (hash_table_contains_key(pq->index_table, elem))
        return 0;

    // ensure there is room for one more element
    if (ensure_capacity(pq) != 0)
        return -1;

    int pos = pq->size; // first free position in the heap array
    pq->heap[pos] = elem;
    pq->size++;

    // allocate a size_t to store the index of this element in the heap
    size_t *idx = malloc(sizeof(size_t));
    if (!idx){
        // rollback: undo the size increment
        pq->size--;
        return -1;
    }

    *idx = (size_t)pos;
    // store mapping: element -> &index
    hash_table_put(pq->index_table, elem, idx);
    if (hash_table_get(pq->index_table, elem) != idx) {
        free(idx);
        pq->size--;
        return -1;
    }

    // restore heap property by moving the element up if needed
    shift_up(pq, pos);
    return 1;
}

int priority_queue_contains(const PriorityQueue *pq, const void *elem){
    if (!pq || !elem)
        return -1;
    return hash_table_contains_key(pq->index_table, elem);
}

void *priority_queue_top(const PriorityQueue *pq){
    if (!pq || pq->size == 0)
        return NULL;
    return pq->heap[0];
}

void priority_queue_pop(PriorityQueue *pq){
    if (!pq || pq->size == 0)
        return;

    // element at root to be removed
    void *root = pq->heap[0];
    size_t *idx_root = hash_table_get(pq->index_table, root);

    // free the stored index and remove entry from the hash table
    if (idx_root)
        free(idx_root);
    hash_table_remove(pq->index_table, root);

    int last = pq->size - 1;

    if (last == 0){
        // only one element in the heap, now removed
        pq->size = 0;
        return;
    }

    // move last element to the root position
    pq->heap[0] = pq->heap[last];
    pq->size--;

    // update its index in the hash table to 0
    size_t *idx_last = hash_table_get(pq->index_table, pq->heap[0]);
    if (idx_last)
        *idx_last = 0;

    // restore heap property by moving the element down
    shift_down(pq, 0);
}

int priority_queue_remove(PriorityQueue *pq, const void *elem){
    if (!pq || !elem)
        return -1;
    if (pq->size == 0)
        return 0;

    if (!hash_table_contains_key(pq->index_table, elem))
        return 0;

    // get the index of the element in the heap
    size_t *idx = hash_table_get(pq->index_table, elem);
    if (!idx)
        return 0;

    int pos = (int)*idx;
    free(idx);
    hash_table_remove(pq->index_table, elem);

    int last = pq->size - 1;

    // if we are removing the last element, just shrink the heap
    if (pos == last){
        pq->size--;
        return 1;
    }

    // move last element into the removed position
    pq->heap[pos] = pq->heap[last];
    pq->size--;

    // update the index of the moved element in the hash table
    size_t *idx_last = hash_table_get(pq->index_table, pq->heap[pos]);
    if (idx_last)
        *idx_last = (size_t)pos;

    // decide whether to restore heap going up or down:
    // if the moved element is greater than its parent, it must go up,
    // otherwise it may need to go down.
    if (pos > 0 && pq->compare(pq->heap[pos], pq->heap[PARENT(pos)]) > 0)
        shift_up(pq, pos);
    else
        shift_down(pq, pos);

    return 1;
}

int priority_queue_size(const PriorityQueue *pq){
    if (!pq)
        return -1;
    return pq->size;
}

static int ensure_capacity(PriorityQueue *pq){
    if (pq->size < pq->capacity)
        return 0; // there is still free space

    if (pq->capacity > INT_MAX / 2) return -1;
    int newcap = (pq->capacity == 0 ? 8 : pq->capacity * 2);
    void **newheap = realloc(pq->heap, (size_t)newcap * sizeof(void *));

    if (!newheap)
        return -1; // allocation error

    pq->heap = newheap;
    pq->capacity = newcap;
    return 0;
}

void priority_queue_free(PriorityQueue *pq){
    if (!pq)
        return;

    /* Release indices directly, without allocating during destruction. */
    for (int i = 0; i < pq->size; i++)
        free(hash_table_get(pq->index_table, pq->heap[i]));

    hash_table_free(pq->index_table);
    free(pq->heap);
    free(pq);
}

static void swap_position(PriorityQueue *pq, int i, int j){
    if (i == j)
        return;

    void *a = pq->heap[i];
    void *b = pq->heap[j];

    pq->heap[i] = b;
    pq->heap[j] = a;

    // update indices in the hash table
    size_t *idxA = hash_table_get(pq->index_table, a);
    size_t *idxB = hash_table_get(pq->index_table, b);

    if (idxA)
        *idxA = (size_t)j;
    if (idxB)
        *idxB = (size_t)i;
}

static void shift_up(PriorityQueue *pq, int pos){
    while (pos > 0){
        int p = PARENT(pos);
        // if current element is not greater than its parent, stop
        if (pq->compare(pq->heap[pos], pq->heap[p]) <= 0)
            break;

        // otherwise swap with parent and continue
        swap_position(pq, pos, p);
        pos = p;
    }
}

static void shift_down(PriorityQueue *pq, int pos){
    int n = pq->size;

    while (1){
        int best = pos;

        // choose the child with the highest priority among up to K children
        for (int i = 0; i < K; i++){
            size_t c = 3 * (size_t)pos + 1 + (size_t)i;
            if (c < (size_t)n && pq->compare(pq->heap[c], pq->heap[best]) > 0)
                best = (int)c;
        }

        // if no child is better, heap property is satisfied
        if (best == pos)
            break;

        // otherwise swap and continue from the new position
        swap_position(pq, pos, best);
        pos = best;
    }
}
