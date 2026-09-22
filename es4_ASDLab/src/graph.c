#include <stdlib.h>
#include <stdint.h>
#include "graph.h"
#include "hash_table.h"

typedef struct adjacency_node {
    void *dest;                  // destination node of the edge
    void *label;                 // edge label (can be NULL if graph is not labelled)
    struct adjacency_node *next; // next edge in the adjacency list
} AdjacencyNode;

struct graph {
    HashTable *adjacency; // hash table: key = node (void*), value = (AdjacencyNode*)

    int labelled; // 1 if the graph is labelled, 0 otherwise
    int directed; // 1 if the graph is directed, 0 otherwise

    int (*compare_nodes)(const void *, const void *); // comparison function for nodes
    unsigned long (*hash_nodes)(const void *);        // hash function for nodes

    size_t num_nodes; // number of nodes currently in the graph
    size_t num_edges; // number of logical edges (an undirected edge is counted once)
};

Graph graph_create(int labelled, int directed,
                   int (*compare)(const void *, const void *),
                   unsigned long (*hash)(const void *)) {
    if (!compare || !hash)
        return NULL;

    // Graph is typedef'd as 'struct graph *', so we allocate the struct
    Graph g = malloc(sizeof(struct graph));
    if (!g)
        return NULL;

    g->labelled = labelled ? 1 : 0;
    g->directed = directed ? 1 : 0;

    g->compare_nodes = compare;
    g->hash_nodes = hash;

    g->adjacency = hash_table_create(compare, hash);
    if (!g->adjacency){
        free(g);
        return NULL;
    }

    g->num_nodes = 0;
    g->num_edges = 0;

    return g;
}

int graph_is_directed(const Graph gr) {
    if (!gr)
        return 0;
    return gr->directed;
}

int graph_is_labelled(const Graph gr) {
    if (!gr)
        return 0;
    return gr->labelled;
}

int graph_add_node(Graph gr, const void *node) {
    if (!gr || !node)
        return 0;

    if (hash_table_contains_key(gr->adjacency, node))
        return 0;

    // Insert the node with an empty adjacency list (NULL)
    hash_table_put(gr->adjacency, node, NULL);

    if (!hash_table_contains_key(gr->adjacency, node)) return 0;
    gr->num_nodes++;
    return 1;
}

int graph_contains_node(const Graph gr, const void *node) {
    if (!gr || !node)
        return 0;

    return hash_table_contains_key(gr->adjacency, node);
}

int graph_remove_node(Graph gr, const void *node) {
    if (!gr || !node)
        return 0;

    if (!hash_table_contains_key(gr->adjacency, node))
        return 0;

    int n = hash_table_size(gr->adjacency);
    void **keys = hash_table_keyset(gr->adjacency);
    if (!keys) return 0;

    // 1. Free the adjacency list (outgoing edges) of this node
    AdjacencyNode *adj = hash_table_get(gr->adjacency, node);
    while (adj) {
        AdjacencyNode *next = adj->next;
        free(adj);
        adj = next;
        gr->num_edges--; // each adjacency entry is an edge
    }

    // Remove the node key from the hash table
    hash_table_remove(gr->adjacency, node);
    gr->num_nodes--;

    // 2. Remove all incoming edges from other nodes
    for (int i = 0; i < n; i++) {
        const void *src = keys[i];
        if (gr->compare_nodes(src, node) == 0) continue;
        AdjacencyNode *curr = hash_table_get(gr->adjacency, src);
        AdjacencyNode *prev = NULL;

        while (curr) {
            if (gr->compare_nodes(curr->dest, node) == 0){
                // remove 'curr' from the adjacency list
                if (!prev)
                    hash_table_put(gr->adjacency, src, curr->next);
                else
                    prev->next = curr->next;

                AdjacencyNode *to_free = curr;
                curr = curr->next;
                free(to_free);

                // For directed graphs, this corresponds to a logical edge.
                // For undirected graphs, we already decremented when
                // removing the adjacency list of 'node'.
                if (gr->directed)
                    gr->num_edges--;
            } else {
                prev = curr;
                curr = curr->next;
            }
        }
    }

    free(keys);
    return 1;
}

int graph_num_nodes(const Graph gr) {
    if (!gr)
        return 0;

    // The number of nodes does NOT depend on whether the graph is directed or not.
    return (int)gr->num_nodes;
}

void **graph_get_nodes(const Graph gr) {
    if (!gr)
        return NULL;

    void **keys = hash_table_keyset(gr->adjacency);
    if (!keys)
        return NULL;

    return keys; // caller must free(keys)
}

int graph_add_edge(Graph gr, const void *node1, const void *node2, const void *label) {
    if (!gr || !node1 || !node2)
        return 0;

    // Both endpoints must exist in the graph
    if (!hash_table_contains_key(gr->adjacency, node1))
        return 0;
    if (!hash_table_contains_key(gr->adjacency, node2))
        return 0;

    if (graph_contains_edge(gr, node1, node2)) return 0;
    AdjacencyNode *forward = malloc(sizeof(*forward));
    if (!forward) return 0;
    AdjacencyNode *reverse = NULL;
    if (!gr->directed && gr->compare_nodes(node1, node2) != 0) {
        reverse = malloc(sizeof(*reverse));
        if (!reverse) { free(forward); return 0; }
        *reverse = (AdjacencyNode){(void *)node1, gr->labelled ? (void *)label : NULL,
                                  hash_table_get(gr->adjacency, node2)};
    }
    *forward = (AdjacencyNode){(void *)node2, gr->labelled ? (void *)label : NULL,
                              hash_table_get(gr->adjacency, node1)};
    hash_table_put(gr->adjacency, node1, forward);
    if (reverse) hash_table_put(gr->adjacency, node2, reverse);
    gr->num_edges++;
    return 1;
}

int graph_contains_edge(const Graph gr, const void *node1, const void *node2) {
    if (!gr || !node1 || !node2)
        return 0;

    AdjacencyNode *curr = hash_table_get(gr->adjacency, node1);
    while (curr) {
        if (gr->compare_nodes(curr->dest, node2) == 0)
            return 1;
        curr = curr->next;
    }

    return 0;
}

int graph_remove_edge(Graph gr, const void *node1, const void *node2){
    if (!gr || !node1 || !node2)
        return 0;

    // Remove edge node1 -> node2
    AdjacencyNode *curr = hash_table_get(gr->adjacency, node1);
    AdjacencyNode *prev = NULL;
    int removed = 0;

    while (curr) {
        if (gr->compare_nodes(curr->dest, node2) == 0) {
            // Remove 'curr' from the adjacency list
            if (prev == NULL)
                hash_table_put(gr->adjacency, node1, curr->next);
            else
                prev->next = curr->next;

            AdjacencyNode *to_free = curr;
            curr = curr->next;
            free(to_free);

            gr->num_edges--;
            removed = 1;
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    if (!removed)
        return 0;

    // If graph is undirected, also remove symmetric edge node2 -> node1
    if (!gr->directed) {
        AdjacencyNode *curr2 = hash_table_get(gr->adjacency, node2);
        AdjacencyNode *prev2 = NULL;

        while (curr2) {
            if (gr->compare_nodes(curr2->dest, node1) == 0) {
                if (prev2 == NULL)
                    hash_table_put(gr->adjacency, node2, curr2->next);
                else
                    prev2->next = curr2->next;

                AdjacencyNode *to_free2 = curr2;
                curr2 = curr2->next;
                free(to_free2);

                break;
            }
            prev2 = curr2;
            curr2 = curr2->next;
        }
    }

    return 1;
}

int graph_num_edges(const Graph gr) {
    if (!gr)
        return 0;
    return (int)gr->num_edges;
}

Edge **graph_get_edges(const Graph gr) {
    if (!gr)
        return NULL;

    int logical_edges = graph_num_edges(gr);
    if (logical_edges == 0)
        return NULL;

    // Allocate array for logical edges
    Edge **result = malloc(sizeof(Edge *) * (size_t)logical_edges);
    if (!result)
        return NULL;

    int n = hash_table_size(gr->adjacency);
    void **keys = hash_table_keyset(gr->adjacency);
    if (!keys){
        free(result);
        return NULL;
    }

    int index = 0;
    for (int i = 0; i < n; i++) {
        void *src = keys[i];
        for (AdjacencyNode *curr = hash_table_get(gr->adjacency, src); curr; curr = curr->next) {
            /* Use key comparison, not pointer identity, to select one direction. */
            if (!gr->directed && gr->compare_nodes(src, curr->dest) > 0) continue;
            Edge *edge = malloc(sizeof(*edge));
            if (!edge) {
                for (int j = 0; j < index; j++) free(result[j]);
                free(result);
                free(keys);
                return NULL;
            }
            *edge = (Edge){src, curr->dest, curr->label};
            result[index++] = edge;
        }
    }
    free(keys);
    return result;
}

void *graph_get_label(const Graph gr, const void *node1, const void *node2){
    if (!gr || !node1 || !node2)
        return NULL;

    AdjacencyNode *curr = hash_table_get(gr->adjacency, node1);

    while (curr){
        if (gr->compare_nodes(curr->dest, node2) == 0)
            return curr->label;
        curr = curr->next;
    }

    return NULL;
}

void **graph_get_neighbours(const Graph gr, const void *node){
    if (!gr || !node)
        return NULL;

    if (!hash_table_contains_key(gr->adjacency, node))
        return NULL;

    AdjacencyNode *curr = hash_table_get(gr->adjacency, node);
    if (!curr)
        return NULL;

    // Count neighbours
    int count = 0;
    AdjacencyNode *tmp = curr;
    while (tmp){
        count++;
        tmp = tmp->next;
    }

    // Allocate array of neighbours
    void **neighbours = malloc(sizeof(void *) * (size_t)count);
    if (!neighbours)
        return NULL;

    tmp = curr;
    for (int i = 0; i < count; i++){
        neighbours[i] = tmp->dest;
        tmp = tmp->next;
    }

    return neighbours;
}

int graph_num_neighbours(const Graph gr, const void *node){
    if (!gr || !node)
        return 0;

    if (!hash_table_contains_key(gr->adjacency, node))
        return 0;

    AdjacencyNode *curr = hash_table_get(gr->adjacency, node);

    int count = 0;
    while (curr){
        count++;
        curr = curr->next;
    }

    return count;
}

void graph_free(Graph gr){
    if (!gr)
        return;

    if (gr->adjacency){
        int n = hash_table_size(gr->adjacency);
        void **keys = hash_table_keyset(gr->adjacency);

        if (keys){
            for (int i = 0; i < n; i++){
                void *node = keys[i];
                AdjacencyNode *curr = hash_table_get(gr->adjacency, node);

                while (curr){
                    AdjacencyNode *next = curr->next;
                    free(curr);
                    curr = next;
                }
            }
            free(keys);
        }

        hash_table_free(gr->adjacency);
    }

    free(gr);
}
