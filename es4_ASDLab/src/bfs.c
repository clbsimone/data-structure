#include "bfs.h"
#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "hash_table.h"

void **breadth_first_visit(Graph gr,
                           void *start,
                           int (*compare)(const void *, const void *),
                           unsigned long (*hash)(const void *)) {

    /* Basic parameter validation */
    if (!gr || !start || !compare || !hash) return NULL;
    
    /* If the starting node is not in the graph, BFS cannot start */
    if (!graph_contains_node(gr, start)) return NULL;
    
    int n = graph_num_nodes(gr);
    if (n <= 0) return NULL;
    
    /* Output array: BFS visits at most all graph nodes.
     * One extra position is used to place a terminating NULL pointer.
     */
    void **order = malloc(((size_t)n + 1u) * sizeof(void *));
    if (!order) return NULL;
    
    /* BFS queue implemented as a simple array.
     * Max size = number of nodes, since no node is enqueued twice.
     */
    void **queue = malloc((size_t)n * sizeof(void *));
    if (!queue) {
        free(order);
        return NULL;
    }

    int head = 0;   // index of the current front element
    int tail = 0;   // index where the next element will be enqueued

    /* Hash table used to keep track of visited nodes */
    HashTable *visited = hash_table_create(compare, hash);
    if (!visited) {
        free(order);
        free(queue);
        return NULL;
    }

    /* Enqueue the starting node and mark it as visited */
    queue[tail++] = start;
    hash_table_put(visited, start, start);
    if (!hash_table_contains_key(visited, start)) goto failure;
    
    int visited_count = 0;

    /* BFS loop */
    while (head < tail) {
        /* Dequeue the next node */
        void *node = queue[head++];

        /* Store the node in the result array */
        order[visited_count++] = node;

        /* Retrieve all neighbors of the current node */
        int num_neighbours = graph_num_neighbours(gr, node);
        if (num_neighbours > 0) {
            void **neighbours = graph_get_neighbours(gr, node);
            
            if (!neighbours) goto failure;
            if (neighbours) {
                /* Explore each neighbor */
                for (int i = 0; i < num_neighbours; i++) {
                    void *n = neighbours[i];

                    /* If the neighbor was not visited yet:
                     * mark it visited and enqueue it.
                     */
                    if (!hash_table_contains_key(visited, n)) {
                        hash_table_put(visited, n, n);
                        if (!hash_table_contains_key(visited, n)) {
                            free(neighbours);
                            goto failure;
                        }
                        queue[tail++] = n;
                    }
                }

                /* The neighbors array is allocated by the graph library */
                free(neighbours);
            }
        }
    }
    
    /* Cleanup temporary data structures */
    hash_table_free(visited);
    free(queue);
    
    /* Add NULL terminator to the result array */
    if (visited_count <= n)
        order[visited_count] = NULL;
    
    return order;

failure:
    hash_table_free(visited);
    free(queue);
    free(order);
    return NULL;
}
