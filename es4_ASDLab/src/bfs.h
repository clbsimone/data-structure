#ifndef BFS_H_SIMONE_BASSO
#define BFS_H_SIMONE_BASSO

#include "graph.h"

/**
 * @brief Breadth-First Search (BFS) starting from a given node.
 *
 * This function visits all nodes reachable from the starting node using
 * a standard BFS algorithm. It uses a queue and a hash table (for the
 * visited set) internally.
 *
 * @param gr       The graph on which BFS is executed.
 * @param start    The starting node for the visit.
 * @param compare  Function used to compare two node keys (returns 0 if equal).
 * @param hash     Hash function for node keys.
 *
 * @return A dynamically allocated array of (void*) containing the nodes
 *         in the order they are visited by BFS. The last element of the
 *         array is a NULL pointer (sentinel).
 *
 *         Returns NULL if:
 *           - the graph pointer is NULL,
 *           - the start node is NULL,
 *           - compare or hash are NULL,
 *           - the start node is not present in the graph,
 *           - or a memory allocation error occurs.
 *
 * @note The returned array must be freed by the caller using free().
 *       The function does not free or modify the nodes stored in the graph.
 */
void **breadth_first_visit(Graph gr,
                           void *start,
                           int (*compare)(const void *, const void *),
                           unsigned long (*hash)(const void *));

#endif
