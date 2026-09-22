#ifndef GRAPH_H_SIMONE_BASSO
#define GRAPH_H_SIMONE_BASSO

#include <stddef.h>
#include "hash_table.h"

/**
 * @brief Opaque type for the graph structure.
 *
 * The actual structure is defined in graph.c.
 */
typedef struct graph *Graph;

/**
 * @brief Structure representing a graph edge.
 *
 * The graph stores edges using this format when
 * returning results from functions like graph_get_edges().
 */
typedef struct edge {
    void *source; ///< pointer to the source node
    void *dest;   ///< pointer to the destination node
    void *label;  ///< pointer to the edge label (NULL if unlabelled graph)
} Edge;

/**
 * @brief Create a new empty graph.
 *
 * The graph is labelled if @p labelled is 1, and directed if @p directed is 1.
 * The @p compare callback must define a total order; equal keys must hash equally.
 *
 * @return A newly allocated Graph, or NULL on failure.
 */
Graph graph_create(int labelled, int directed,
                   int (*compare)(const void *, const void *),
                   unsigned long (*hash)(const void *));

/**
 * @brief Check whether the graph is directed.
 */
int graph_is_directed(const Graph gr);

/**
 * @brief Check whether the graph is labelled.
 */
int graph_is_labelled(const Graph gr);

/**
 * @brief Add a new node to the graph.
 *
 * @return 1 on success, 0 on failure or if the node already exists.
 */
int graph_add_node(Graph gr, const void *node);

/**
 * @brief Check whether a node exists in the graph.
 */
int graph_contains_node(const Graph gr, const void *node);

/**
 * @brief Remove a node and all incident edges.
 *
 * @return 1 on success, 0 for invalid input, a missing node, or allocation failure.
 */
int graph_remove_node(Graph gr, const void *node);

/**
 * @brief Get the number of nodes in the graph.
 */
int graph_num_nodes(const Graph gr);

/**
 * @brief Get all nodes of the graph.
 *
 * @return An array of node pointers. Must be freed by the caller.
 */
void **graph_get_nodes(const Graph gr);

/**
 * @brief Add an edge from node1 to node2 with the given label.
 *
 * For undirected graphs, a symmetric edge is automatically added.
 *
 * @return 1 on success, 0 for invalid input, missing nodes, duplicates, or allocation failure.
 */
int graph_add_edge(Graph gr, const void *node1, const void *node2, const void *label);

/**
 * @brief Check whether an edge from node1 to node2 exists.
 */
int graph_contains_edge(const Graph gr, const void *node1, const void *node2);

/**
 * @brief Remove the edge from node1 to node2.
 *
 * In undirected graphs, also removes the symmetric edge.
 *
 * @return 1 on success, 0 if the edge does not exist.
 */
int graph_remove_edge(Graph gr, const void *node1, const void *node2);

/**
 * @brief Get the total number of edges in the graph.
 *
 * For undirected graphs, each logical edge is counted once, including self-loops.
 */
int graph_num_edges(const Graph gr);

/**
 * @brief Get all edges of the graph.
 *
 * @return Array of Edge* pointers. Caller must free the array and the Edge objects.
 */
Edge **graph_get_edges(const Graph gr);

/**
 * @brief Retrieve the label of the edge (node1 -> node2).
 *
 * @return Pointer to the label, or NULL if edge does not exist or graph is unlabelled.
 */
void *graph_get_label(const Graph gr, const void *node1, const void *node2);

/**
 * @brief Get all neighbours of a given node.
 *
 * @return Array of node pointers. Must be freed by the caller.
 */
void **graph_get_neighbours(const Graph gr, const void *node);

/**
 * @brief Get the number of neighbours of a given node.
 */
int graph_num_neighbours(const Graph gr, const void *node);

/**
 * @brief Free all memory associated with the graph.
 *
 * Does NOT free node objects or labels — ownership stays with the user.
 */
void graph_free(Graph gr);

#endif // GRAPH_H
