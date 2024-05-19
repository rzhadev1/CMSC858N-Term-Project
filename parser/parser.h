#ifndef _PARSER_H
#define _PARSER_H
#include <iostream>
#include <string>
#include <map>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/io.h>

using vertex_id = int; 
using weight = int; 
using edges = parlay::sequence<std::pair<vertex_id, weight>>; 
using weighted_graph = parlay::sequence<edges>;

struct FlowInstance {
	int n; // number of nodes
	int m; // number of edges
	vertex_id source; 
	vertex_id sink;
	weighted_graph wgh_graph;	// adjacency list representation of the graph
};

// Read the dimacs file to a parlay adjacency list representation
FlowInstance readDimacsToFlowInstance(const std::string& filename);

#endif
