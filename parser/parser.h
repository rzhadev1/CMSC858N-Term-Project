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
using edge = std::pair<vertex_id, vertex_id>;

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

// A hasher for an edge

struct EdgeHasher
{
  std::size_t operator()(const edge& k) const
  {
		std::size_t seed = 0;
		hash_combine(seed, k.first); 
		hash_combine(seed, k.second);

		return seed;
  }
};

struct FlowInstance {
	long n; // number of nodes
	long m; // number of edges
	vertex_id source; 
	vertex_id sink;
	weighted_graph wgh_graph;	// adjacency list representation of the graph
};

FlowInstance readDimacsToFlowInstanceSeq(const std::string& filename);

FlowInstance readDimacsToFlowInstanceParHashMap(const std::string& filename);

FlowInstance readDimacsToFlowInstanceSeqHashTable(const std::string& filename);

FlowInstance readDimacsToFlowInstanceSeqOpt(const std::string& filename);
#endif
