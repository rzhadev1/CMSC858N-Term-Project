#ifndef _SYNC_PAR_H
#define _SYNC_PAR_H
#include <chrono>
#include <limits>
#include <atomic>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/internal/get_time.h>

#include "parser.h"
#include "BFS.h"

// TODO: 
// - switch to sequential when active set is small?

using vertex_id = int; 
using weight = int; 

// adjacency list representation of the flow network
using edges = parlay::sequence<std::pair<vertex_id, weight>>; 
using weighted_graph = parlay::sequence<edges>;

// FIFO synchronous parallel push relabel from Baumstark et. al
class FIFOSyncParPR {
	
	struct Edge {
		int capacity;
		int flow;
		Edge* res_edge; // residual edge
		vertex_id u; 
		vertex_id v;
	}; 

	struct Vertex {
		int label; // label of the node, d(v)
		int excess; // amount of excess at this node, e(v) 
		std::atomic<int> addedExcess; // a copy variable to avoid data races
		std::atomic<bool> isDiscovered;
		parlay::sequence<Edge> edges;

		Vertex() { 
			label = 0; 
			excess = 0;
			addedExcess = 0;
			isDiscovered = false;
		}
	};


	int n; 
	int m; 
	vertex_id sink; 
	vertex_id source;

	parlay::sequence<Vertex> vertices; // vertices indexed by their id
	parlay::sequence<Vertex> active; // all vertices that have positive excess
	
	// init the flow network 
	void init(FlowInstance fi);
	
	// do a global relabel: run a reverse BFS from sink
	// to get exact distance labels
	void global_relabel(); 

	// push flow
	void push();

	// relabel a vertex
	void relabel(); 
	
	// construct a flow solver
	FIFOSyncParPR(FlowInstance fi);

	public: 
		static int solve(FlowInstance fi);
};

#endif
