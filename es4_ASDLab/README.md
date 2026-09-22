# Exercise 4 — Sparse Graphs and Breadth-First Search

A generic graph library supporting directed or undirected graphs, with optional edge labels. A hash table maps nodes to linked adjacency lists. Isolated nodes are represented by keys with NULL adjacency lists.

## Build and usage

```sh
make
make test
./bin/main_ex4 ../examples/places.csv a /tmp/bfs.txt
```

Arguments: input CSV, starting place, and output path. CSV rows contain `place1,place2,distance`. The application builds an undirected, labelled graph, trims surrounding whitespace, and normalizes names to lowercase. Distances must be finite, nonnegative floating-point values. Invalid rows are skipped; duplicate edges keep their first label. Input uses a 1,024-byte line buffer and simple unquoted comma-separated fields.

Output contains one reachable place per line, starting with the requested place. BFS follows adjacency-list order, so nodes at the same distance need not appear alphabetically. A missing starting place or an output error returns failure.

BFS ignores distance labels: it explores by number of edges, not by total geographic distance.

## Graph API

Both endpoints must exist before adding an edge. Duplicate nodes and edges are rejected. An undirected edge is counted once, though non-loop edges have two adjacency entries. Self-loops are supported. Removing a node removes all incident edges.

Keys must use consistent hashing and a total-order comparison callback. Nodes, edge endpoints, and labels are borrowed and must remain valid while referenced. `graph_free` releases container storage, not caller data. The demo frees each distance label once before releasing graph and place-name storage.

Arrays returned by `graph_get_nodes` and `graph_get_neighbours` must be freed by the caller. For `graph_get_edges`, free each returned `Edge` and then the array; node and label pointers inside those wrappers remain borrowed. `breadth_first_visit` returns a caller-owned, NULL-terminated array of borrowed node pointers, or NULL for invalid input or allocation failure.

## Complexity

Storage is O(V + E). Node lookup is expected O(1). Edge lookup, insertion, and removal depend on the source vertex degree; undirected operations also inspect or update the destination list. Removing a node scans the graph and takes expected O(V + E). Building an arbitrary graph can exceed O(V + E) because duplicate detection scans adjacency lists.

BFS takes expected O(V + E) time with well-distributed hashes and O(V) extra space. It returns only the starting node's reachable component.

## Tests

Unity tests cover graph flags, isolated and duplicate nodes, directed incident-edge removal, undirected symmetry, labels, self-loops, logically equal keys at different addresses, BFS validation, and disconnected components. Root-level CLI tests additionally verify traversal layers and missing starting places.
