#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

#include "graph.h"
#include "hash_table.h"
#include "bfs.h"

static char *copy_string(const char *source) {
    size_t size = strlen(source) + 1;
    char *copy = malloc(size);
    if (copy) memcpy(copy, source, size);
    return copy;
}


#define LINE_BUFFER_SIZE 1024

/*
 * Compare two C strings (used by graph and hash table).
 */
static int string_compare(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

static void str_to_lower(char *s) {
    while (*s) {
        *s = (char)tolower((unsigned char)*s);
        s++;
    }
}

/*
 * String hash function (djb2).
 *
 * Classic hash: hash = hash * 33 + c
 */
static unsigned long string_hash(const void *k) {
    const unsigned char *str = (const unsigned char *)k;
    unsigned long hash = 5381;
    int c;

    while ((c = *str++) != 0) {
        // IMPORTANT: parentheses matter here!
        hash = ((hash << 5) + hash) + (unsigned long)c; /* hash * 33 + c */
    }

    return hash;
}

static char *trim_whitespace(char *s) {
    char *end;

    if (!s)
        return s;

    // Skip leading whitespace characters
    while (isspace((unsigned char)*s)) {
        s++;
    }

    // If the string is now empty, nothing more to do
    if (*s == '\0')
        return s;

    // Move 'end' to the last character of the string
    end = s + strlen(s) - 1;

    // Move backwards while there is trailing whitespace
    while (end > s && isspace((unsigned char)*end)) {
        *end = '\0';  /* cut the string here */
        end--;
    }

    return s;
}

static Graph build_graph_from_file(const char *filename, HashTable **node_index_out) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Error opening input CSV file");
        *node_index_out = NULL;
        return NULL;
    }

    /* Create an undirected, labelled graph:
     * labelled = 1 (edges store a label, here a float* distance)
     * directed = 0 (edges are considered undirected)
     */
    Graph gr = graph_create(1, 0, string_compare, string_hash);
    if (!gr) {
        fclose(f);
        *node_index_out = NULL;
        return NULL;
    }

    // Hash table: place name -> canonical string pointer used in the graph
    HashTable *node_index = hash_table_create(string_compare, string_hash);
    if (!node_index) {
        graph_free(gr);
        fclose(f);
        *node_index_out = NULL;
        return NULL;
    }

    char line[LINE_BUFFER_SIZE];

    // Read the CSV file line by line
    while (fgets(line, sizeof(line), f) != NULL) {
        // Split the line into tokens:  place1, place2, distance
        char *place1 = strtok(line, ",");
        char *place2 = strtok(NULL, ",");
        char *dist_s = strtok(NULL, "\n");  // read until end of line

        // If some field is missing, skip the line
        if (!place1 || !place2 || !dist_s)
            continue;

        // Clean up spaces around strings
        place1 = trim_whitespace(place1);
        place2 = trim_whitespace(place2);
        dist_s = trim_whitespace(dist_s);

        str_to_lower(place1);
        str_to_lower(place2);
        
        // Convert distance string to float
        char *endptr = NULL;
        errno = 0;
        float dist_value = strtof(dist_s, &endptr);

        // If no valid float was read, skip the line (bad or header row)
        if (errno || endptr == dist_s || *endptr || !isfinite(dist_value) || dist_value < 0 || !*place1 || !*place2)
            continue;

        // Look up or create node1 (place1) in the graph
        char *node1 = (char *)hash_table_get(node_index, place1);
        if (!node1) {
            // Allocate a copy of the place name
            node1 = copy_string(place1);
            if (!node1)
                // Out of memory: skip this line
                continue;
            
                // Add node to graph; on failure, free the name and skip
            if (!graph_add_node(gr, node1)) {
                free(node1);
                continue;
            }
            // Remember this pointer in the index
            hash_table_put(node_index, node1, node1);
        }

        // Look up or create node2 (place2) in the graph
        char *node2 = (char *)hash_table_get(node_index, place2);
        if (!node2) {
            node2 = copy_string(place2);
            if (!node2)
                continue;
            if (!graph_add_node(gr, node2)) {
                free(node2);
                continue;
            }
            hash_table_put(node_index, node2, node2);
        }

        // Allocate an edge label to store the distance
        float *label = (float *)malloc(sizeof(float));
        if (!label)
            // Out of memory: skip this edge
            continue;
        
        *label = dist_value;

        /* Add an undirected edge between node1 and node2.
         * If the graph refuses the edge, we must free the label.
         */
        if (!graph_add_edge(gr, node1, node2, label)) {
            free(label);
        }
    }

    fclose(f);
    *node_index_out = node_index;
    return gr;
}

static int run_bfs_and_write_output(Graph gr, HashTable *index,
                                    const char *s_name, const char *out_file) {
    char *start_name = copy_string(s_name);
    if (!start_name) return 0;
    str_to_lower(start_name);
    char *start_node = hash_table_get(index, start_name);
    free(start_name);
    if (!start_node) {
        fprintf(stderr, "Error: start place '%s' not found in graph.\n", s_name);
        return 0;
    }
    void **order = breadth_first_visit(gr, start_node, string_compare, string_hash);
    if (!order) return 0;
    FILE *f = fopen(out_file, "w");
    if (!f) { free(order); perror("Error opening output file"); return 0; }
    int ok = 1;
    for (size_t i = 0; order[i]; i++) {
        if (fprintf(f, "%s\n", (char *)order[i]) < 0) { ok = 0; break; }
    }
    free(order);
    if (fclose(f) != 0) ok = 0;
    return ok;
}

static void free_place_names(HashTable *ht) {
    int size = hash_table_size(ht);
    void **keys = hash_table_keyset(ht);

    if (!keys)
        return;

    for (int i = 0; i < size; i++)
        free(keys[i]);

    free(keys);
}

int main(int argc, char const **argv) {
    if (argc != 4) {
        // Print usage message to stderr
        fprintf(stderr, "Usage: %s <input_csv> <start_city> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_csv   = argv[1];
    const char *start_city  = argv[2];
    const char *output_file = argv[3];

    HashTable *node_index = NULL;
    Graph gr = build_graph_from_file(input_csv, &node_index);

    if (!gr || !node_index) {
        fprintf(stderr, "Error: could not build graph.\n");
        return EXIT_FAILURE;
    }

    int ok = run_bfs_and_write_output(gr, node_index, start_city, output_file);

    /* Each logical edge owns one label, shared by its two adjacency entries. */
    Edge **edges = graph_get_edges(gr);
    if (edges) {
        for (int i = 0; i < graph_num_edges(gr); i++) {
            free(edges[i]->label);
            free(edges[i]);
        }
        free(edges);
    }
    graph_free(gr);
    free_place_names(node_index);
    hash_table_free(node_index);
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
