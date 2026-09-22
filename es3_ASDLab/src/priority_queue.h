#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stddef.h> // for size_t

/**
 * @brief Opaque structure representing a priority queue.
 *
 * The internal implementation uses a ternary max-heap and a hash table
 * to support efficient contains/remove operations.
 *
 * Elements are stored as void* pointers and compared using the
 * user-provided compare function.
 */
typedef struct PriorityQueue PriorityQueue;

/**
 * @brief Create a new priority queue.
 *
 * The priority queue is implemented as a ternary max-heap.
 *
 * @param compare Function used to compare two elements.
 *                Must return >0 if a has higher priority than b,
 *                0  if a and b represent the same element,
 *                <0 if a has lower priority than b.
 *                Use a tie-breaker for distinct elements with equal priorities.
 *                Elements must remain unchanged while stored.
 * @param hash_f  Hash function for the elements. It is used as the
 *                key hash in the internal hash table (for contains/remove).
 *
 * @return A pointer to a new PriorityQueue, or NULL on failure.
 */
PriorityQueue *priority_queue_create(int (*compare)(const void *, const void *),
                                     unsigned long (*hash_f)(const void *));

/**
 * @brief Insert an element into the priority queue.
 *
 * @param pq   Pointer to the priority queue.
 * @param elem Pointer to the element to insert.
 *
 * @return -1 on error (invalid arguments or allocation failure),
 *          0 if the element was already present (no insertion),
 *          1 if the element has been successfully inserted.
 */
int priority_queue_push(PriorityQueue *pq, void *elem);

/**
 * @brief Check whether an element is contained in the priority queue.
 *
 * @param pq   Pointer to the priority queue.
 * @param elem Pointer to the element to search for.
 *
 * @return -1 on invalid arguments,
 *          1 if the element is present,
 *          0 if the element is not present.
 */
int priority_queue_contains(const PriorityQueue *pq, const void *elem);

/**
 * @brief Get the element with maximum priority (the root of the heap).
 *
 * The queue is not modified by this operation.
 *
 * @param pq Pointer to the priority queue.
 *
 * @return Pointer to the element with maximum priority,
 *         or NULL if the queue is empty or pq is NULL.
 */
void *priority_queue_top(const PriorityQueue *pq);

/**
 * @brief Remove the element with maximum priority from the queue.
 *
 * If the queue is empty or pq is NULL, this function does nothing.
 *
 * @param pq Pointer to the priority queue.
 */
void priority_queue_pop(PriorityQueue *pq);

/**
 * @brief Remove a specific element from the priority queue.
 *
 * @param pq   Pointer to the priority queue.
 * @param elem Pointer to the element to remove.
 *
 * @return -1 on invalid arguments,
 *          0 if the element was not found,
 *          1 if the element has been removed.
 */
int priority_queue_remove(PriorityQueue *pq, const void *elem);

/**
 * @brief Get the number of elements stored in the priority queue.
 *
 * @param pq Pointer to the priority queue.
 *
 * @return The number of elements in the queue, or -1 if pq is NULL.
 */
int priority_queue_size(const PriorityQueue *pq);

/**
 * @brief Free the entire priority queue.
 *
 * This function frees all internal data structures of the priority queue
 * (heap array, hash table, index pointers), but DOES NOT free the elements
 * stored in the queue themselves. Managing the lifetime of the elements
 * is the caller's responsibility.
 *
 * @param pq Pointer to the priority queue to destroy.
 */
void priority_queue_free(PriorityQueue *pq);

#endif /* PRIORITY_QUEUE_H */
