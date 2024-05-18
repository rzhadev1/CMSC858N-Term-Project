#include <iostream>
#include <string>
#include <map>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/io.h>

typedef int vertex_id; 
typedef int weight;
typedef parlay::sequence<std::pair<vertex_id, weight>> edge;
typedef parlay::sequence<edge> weighted_graph;

using vertex_id = int; 
using weight = int; 
using edges = parlay::sequence<std::pair<vertex_id, weight>>; 
using weighted_graph = parlay::sequence<edges>;

struct FlowInstance {
	int n; // number of nodes
	int m; // number of edges
	vertex_id source; 
	vertex_id sink;
	weighted_graph adj_list;	// adjacency list representation of the graph
};

// Read the dimacs file to a parlay adjacency list representation
FlowInstance readDimacsToFlowInstance(const std::string& filename);

