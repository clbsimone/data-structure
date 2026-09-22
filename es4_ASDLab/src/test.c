#include "unity.h"

#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "bfs.h"

static char *copy_string(const char *source) {
    size_t size = strlen(source) + 1;
    char *copy = malloc(size);
    if (copy) memcpy(copy, source, size);
    return copy;
}


/* ---------------- compare/hash for C strings (same idea as main.c) ---------------- */
   
static int string_compare(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

static unsigned long string_hash(const void *k) {
    const unsigned char *str = (const unsigned char *)k;
    unsigned long hash = 5381;
    int c;

    while ((c = *str++) != 0) {
        hash = ((hash << 5) + hash) + (unsigned long)c; /* hash * 33 + c */
    }

    return hash;
}

/* ---------------- Small utilities for checking BFS output arrays (NULL-terminated) ---------------- */

static size_t ptr_array_len(void **arr) {
    size_t n = 0;
    if (!arr) return 0;
    while (arr[n] != NULL) n++;
    return n;
}

static int ptr_array_contains_str(void **arr, const char *s) {
    if (!arr || !s) return 0;
    for (size_t i = 0; arr[i] != NULL; i++) {
        if (strcmp((const char *)arr[i], s) == 0) return 1;
    }
    return 0;
}


void setUp(void) {}
void tearDown(void) {}

/* ---------------- Graph creation & flags ---------------- */

static void test_graph_create_flags(void) {
    Graph g1 = graph_create(1, 0, string_compare, string_hash);
    TEST_ASSERT_NOT_NULL(g1);
    TEST_ASSERT_EQUAL_INT(1, graph_is_labelled(g1));
    TEST_ASSERT_EQUAL_INT(0, graph_is_directed(g1));
    graph_free(g1);

    Graph g2 = graph_create(0, 1, string_compare, string_hash);
    TEST_ASSERT_NOT_NULL(g2);
    TEST_ASSERT_EQUAL_INT(0, graph_is_labelled(g2));
    TEST_ASSERT_EQUAL_INT(1, graph_is_directed(g2));
    graph_free(g2);
}

static void test_graph_create_invalid_params(void) {
    Graph g;

    g = graph_create(1, 1, NULL, string_hash);
    TEST_ASSERT_NULL(g);

    g = graph_create(1, 1, string_compare, NULL);
    TEST_ASSERT_NULL(g);
}

/* ---------------- Node management ---------------- */

static void test_nodes_add_contains_get_count(void) {
    Graph g = graph_create(0, 1, string_compare, string_hash);
    TEST_ASSERT_NOT_NULL(g);

    char *a = copy_string("a");
    char *b = copy_string("b");
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);

    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, a));
    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, b));
    TEST_ASSERT_EQUAL_INT(0, graph_add_node(g, a)); /* duplicate */

    TEST_ASSERT_EQUAL_INT(1, graph_contains_node(g, a));
    TEST_ASSERT_EQUAL_INT(1, graph_contains_node(g, b));
    TEST_ASSERT_EQUAL_INT(2, graph_num_nodes(g));

    void **nodes = graph_get_nodes(g);
    TEST_ASSERT_NOT_NULL(nodes);
    /* Do not assume a particular keyset order. */
    free(nodes);

    graph_free(g);

    /* Nodes are owned by the caller, not by graph_free. */
    free(a);
    free(b);
}

static void test_remove_node_removes_incident_edges_directed(void) {
    Graph g = graph_create(0, 1, string_compare, string_hash);
    TEST_ASSERT_NOT_NULL(g);

    char *a = copy_string("a");
    char *b = copy_string("b");
    char *c = copy_string("c");
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_NOT_NULL(c);

    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, a));
    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, b));
    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, c));

    /* a->b, c->b, b->a */
    TEST_ASSERT_EQUAL_INT(1, graph_add_edge(g, a, b, NULL));
    TEST_ASSERT_EQUAL_INT(1, graph_add_edge(g, c, b, NULL));
    TEST_ASSERT_EQUAL_INT(1, graph_add_edge(g, b, a, NULL));

    /* Before removing the node. */
    TEST_ASSERT_EQUAL_INT(1, graph_contains_edge(g, a, b));
    TEST_ASSERT_EQUAL_INT(1, graph_contains_edge(g, c, b));
    TEST_ASSERT_EQUAL_INT(1, graph_contains_edge(g, b, a));

    /* Removing b must remove all its incoming and outgoing edges. */
    TEST_ASSERT_EQUAL_INT(1, graph_remove_node(g, b));
    TEST_ASSERT_EQUAL_INT(0, graph_contains_node(g, b));

    TEST_ASSERT_EQUAL_INT(0, graph_contains_edge(g, a, b));
    TEST_ASSERT_EQUAL_INT(0, graph_contains_edge(g, c, b));

    /* The removed node must have no remaining outgoing edges. */
    TEST_ASSERT_EQUAL_INT(0, graph_contains_edge(g, b, a));

    graph_free(g);
    free(a); free(b); free(c);
}

/* ---------------- Edge management (directed) ---------------- */

static void test_edges_directed_add_contains_remove(void) {
    Graph g = graph_create(0, 1, string_compare, string_hash);
    TEST_ASSERT_NOT_NULL(g);

    char *a = copy_string("a");
    char *b = copy_string("b");
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);

    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, a));
    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, b));

    TEST_ASSERT_EQUAL_INT(1, graph_add_edge(g, a, b, NULL));
    TEST_ASSERT_EQUAL_INT(0, graph_add_edge(g, a, b, NULL)); /* duplicate */

    TEST_ASSERT_EQUAL_INT(1, graph_contains_edge(g, a, b));
    TEST_ASSERT_EQUAL_INT(0, graph_contains_edge(g, b, a));

    TEST_ASSERT_EQUAL_INT(1, graph_remove_edge(g, a, b));
    TEST_ASSERT_EQUAL_INT(0, graph_contains_edge(g, a, b));
    TEST_ASSERT_EQUAL_INT(0, graph_remove_edge(g, a, b)); /* already removed */

    graph_free(g);
    free(a);
    free(b);
}

static void test_edges_require_existing_nodes(void) {
    Graph g = graph_create(0, 1, string_compare, string_hash);
    TEST_ASSERT_NOT_NULL(g);

    char *a = copy_string("a");
    char *b = copy_string("b");
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);

    /* Adding an edge without endpoints must fail. */
    TEST_ASSERT_EQUAL_INT(0, graph_add_edge(g, a, b, NULL));

    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, a));
    /* Adding an edge with one missing endpoint must fail. */
    TEST_ASSERT_EQUAL_INT(0, graph_add_edge(g, a, b, NULL));

    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, b));
    /* Both endpoints exist, so insertion must succeed. */
    TEST_ASSERT_EQUAL_INT(1, graph_add_edge(g, a, b, NULL));

    graph_free(g);
    free(a);
    free(b);
}

/* ---------------- Edge management (undirected + labelled) ---------------- */

static void test_edges_undirected_labelled_symmetry_and_label(void) {
    Graph g = graph_create(1, 0, string_compare, string_hash);
    TEST_ASSERT_NOT_NULL(g);

    char *a = copy_string("a");
    char *b = copy_string("b");
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);

    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, a));
    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, b));

    float *lab = malloc(sizeof(float));
    TEST_ASSERT_NOT_NULL(lab);
    *lab = 12.5f;

    TEST_ASSERT_EQUAL_INT(1, graph_add_edge(g, a, b, lab));

    /* An undirected edge must be present in both directions. */
    TEST_ASSERT_EQUAL_INT(1, graph_contains_edge(g, a, b));
    TEST_ASSERT_EQUAL_INT(1, graph_contains_edge(g, b, a));

    /* Both directions must share the same label pointer. */
    TEST_ASSERT_EQUAL_PTR(lab, graph_get_label(g, a, b));
    TEST_ASSERT_EQUAL_PTR(lab, graph_get_label(g, b, a));

    /* neighbours */
    TEST_ASSERT_EQUAL_INT(1, graph_num_neighbours(g, a));
    void **na = graph_get_neighbours(g, a);
    TEST_ASSERT_NOT_NULL(na);
    TEST_ASSERT_EQUAL_PTR(b, na[0]);
    free(na);

    /* Removal must also remove the reverse edge. */
    TEST_ASSERT_EQUAL_INT(1, graph_remove_edge(g, a, b));
    TEST_ASSERT_EQUAL_INT(0, graph_contains_edge(g, a, b));
    TEST_ASSERT_EQUAL_INT(0, graph_contains_edge(g, b, a));

    /* Free the caller-owned label. */
    free(lab);

    graph_free(g);
    free(a);
    free(b);
}

/* ---------------- BFS behaviour ---------------- */

static void test_bfs_invalid_params(void) {
    Graph g = graph_create(0, 1, string_compare, string_hash);
    TEST_ASSERT_NOT_NULL(g);

    char *a = copy_string("a");
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, a));

    TEST_ASSERT_NULL(breadth_first_visit(NULL, a, string_compare, string_hash));
    TEST_ASSERT_NULL(breadth_first_visit(g, NULL, string_compare, string_hash));
    TEST_ASSERT_NULL(breadth_first_visit(g, a, NULL, string_hash));
    TEST_ASSERT_NULL(breadth_first_visit(g, a, string_compare, NULL));

    graph_free(g);
    free(a);
}

static void test_bfs_start_not_in_graph(void) {
    Graph g = graph_create(0, 1, string_compare, string_hash);
    TEST_ASSERT_NOT_NULL(g);

    char *a = copy_string("a");
    char *x = copy_string("x");
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(x);

    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, a));

    void **order = breadth_first_visit(g, x, string_compare, string_hash);
    TEST_ASSERT_NULL(order);

    graph_free(g);
    free(a);
    free(x);
}

static void test_bfs_reachable_set_and_terminator(void) {
    /* Undirected graph: a-b, a-c, b-d; e is isolated. */
    Graph g = graph_create(0, 0, string_compare, string_hash);
    TEST_ASSERT_NOT_NULL(g);

    char *a = copy_string("a");
    char *b = copy_string("b");
    char *c = copy_string("c");
    char *d = copy_string("d");
    char *e = copy_string("e"); /* isolated */
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_NOT_NULL(c);
    TEST_ASSERT_NOT_NULL(d);
    TEST_ASSERT_NOT_NULL(e);

    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, a));
    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, b));
    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, c));
    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, d));
    TEST_ASSERT_EQUAL_INT(1, graph_add_node(g, e));

    TEST_ASSERT_EQUAL_INT(1, graph_add_edge(g, a, b, NULL));
    TEST_ASSERT_EQUAL_INT(1, graph_add_edge(g, a, c, NULL));
    TEST_ASSERT_EQUAL_INT(1, graph_add_edge(g, b, d, NULL));

    void **order = breadth_first_visit(g, a, string_compare, string_hash);
    TEST_ASSERT_NOT_NULL(order);

    /* The starting node must appear first. */
    TEST_ASSERT_EQUAL_PTR(a, order[0]);

    /* The result must be NULL-terminated. */
    size_t len = ptr_array_len(order);
    TEST_ASSERT_TRUE(len > 0);
    TEST_ASSERT_NULL(order[len]);

    /* The result must contain exactly {a,b,c,d}, excluding e. */
    TEST_ASSERT_EQUAL_UINT64(4u, (uint64_t)len);

    TEST_ASSERT_TRUE(ptr_array_contains_str(order, "a"));
    TEST_ASSERT_TRUE(ptr_array_contains_str(order, "b"));
    TEST_ASSERT_TRUE(ptr_array_contains_str(order, "c"));
    TEST_ASSERT_TRUE(ptr_array_contains_str(order, "d"));
    TEST_ASSERT_FALSE(ptr_array_contains_str(order, "e"));

    free(order);
    graph_free(g);
    free(a); free(b); free(c); free(d); free(e);
}

/* ---------------- main() ---------------- */
   
static void test_undirected_self_loop_and_equal_keys(void) {
    Graph g = graph_create(0, 0, string_compare, string_hash);
    char a[] = "a", b[] = "b", alias[] = "b";
    TEST_ASSERT_TRUE(graph_add_node(g, a));
    TEST_ASSERT_TRUE(graph_add_node(g, b));
    TEST_ASSERT_TRUE(graph_add_edge(g, a, alias, NULL));
    TEST_ASSERT_TRUE(graph_add_edge(g, a, a, NULL));
    TEST_ASSERT_EQUAL_INT(2, graph_num_edges(g));
    Edge **edges = graph_get_edges(g);
    TEST_ASSERT_NOT_NULL(edges);
    for (int i = 0; i < 2; i++) { TEST_ASSERT_NOT_NULL(edges[i]); free(edges[i]); }
    free(edges);
    TEST_ASSERT_TRUE(graph_remove_node(g, a));
    TEST_ASSERT_EQUAL_INT(0, graph_num_edges(g));
    TEST_ASSERT_EQUAL_INT(0, graph_num_neighbours(g, b));
    graph_free(g);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_undirected_self_loop_and_equal_keys);

    RUN_TEST(test_graph_create_flags);
    RUN_TEST(test_graph_create_invalid_params);

    RUN_TEST(test_nodes_add_contains_get_count);
    RUN_TEST(test_remove_node_removes_incident_edges_directed);

    RUN_TEST(test_edges_directed_add_contains_remove);
    RUN_TEST(test_edges_require_existing_nodes);
    RUN_TEST(test_edges_undirected_labelled_symmetry_and_label);

    RUN_TEST(test_bfs_invalid_params);
    RUN_TEST(test_bfs_start_not_in_graph);
    RUN_TEST(test_bfs_reachable_set_and_terminator);

    return UNITY_END();
}
