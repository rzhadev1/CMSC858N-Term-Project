#ifndef _SYNC_PAR_H
#define _SYNC_PAR_H
#include <atomic>

#include <parlay/primitives.h>
#include <parlay/sequence.h>

#include "parser.h"

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
		int res_capacity; // residual edge capacity
		Edge* res_edge; // residual edge
		vertex_id u; 
		vertex_id v;
	}; 

	struct Vertex {
		int label; // label of the node, d(v)
		int newlabel; // copy of label to avoid data races
		int excess; // amount of excess at this node, e(v) 
		std::atomic<int> addedExcess; // a copy variable to avoid data races
		std::atomic<bool> isDiscovered; // whether or not this vertex was added to the active set
		int work; // the amount of "work" (pushes) done by this vertex
		int current; // the last edge that was pushed on by this vertex

		parlay::sequence<Edge> edges;
		int outDegree; // the number of outgoing edges at this vertex
		parlay::sequence<vertex_id> discoveredVertices; // used in global relabeling/pushs
		
		Vertex() : label(0), newlabel(0), excess(0), addedExcess(0), current(0), isDiscovered(false), work(0) {}
	};

	int n; 
	int m; 
	vertex_id sink; 
	vertex_id source;

	parlay::sequence<Vertex> vertices; // vertices indexed by their id
	parlay::sequence<vertex_id> active; // all vertex ids that have positive excess
	
	std::chrono::duration<double> relabel_time;

	// global relabeling parameters
	int workSinceLastGR;
	const float freq = 0.5; 
	const int alpha = 6;
	const int beta = 12;

	// init the flow network 
	void init(FlowInstance fi);
	
	// do a global relabel: run a reverse BFS from sink
	// to get exact distance labels
	void global_relabel(); 

	// push flow
	void push(vertex_id ui, bool& need_relabel);
	
	// try pushing a vertex v to the argument sequence
	// use this push to a local vertex queue of active pushes that are made in the next round
	void push_active(parlay::sequence<vertex_id> &a, vertex_id vi);

	// relabel a vertex
	void relabel(vertex_id ui); 
	
	// construct a flow solver
	FIFOSyncParPR(FlowInstance fi);
	
	// from the push_relabel example on parlaylib
	// check the correctness of the flow according to the correctness proof of push relabel 
	// - check that flow matches excess at each vertex
	// - no flow exceeds capacity
	// - no invalid labeling between vertices
	// - no left over excess
	void check_correctness();

	public: 
		static int solve(FlowInstance fi);
};

#endif
