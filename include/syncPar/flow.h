#ifndef _SYNC_PAR_H
#define _SYNC_PAR_H
#include <atomic>

#include <parlay/primitives.h>
#include <parlay/sequence.h>

#include "parser.h"

using vertex_id = int; 

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
		int outDegree; // the number of outgoing edges at this vertex
		parlay::sequence<Edge> edges;
		parlay::sequence<vertex_id> discoveredVertices; // used in global relabeling/pushs
		
		Vertex() : label(0), newlabel(0), excess(0), addedExcess(0), current(0), isDiscovered(false), work(0) {}
	};

	int n; 
	int m; 
	vertex_id sink; 
	vertex_id source;

	parlay::sequence<Vertex> vertices; // vertices indexed by their id
	parlay::sequence<vertex_id> active; // all vertex ids that have positive excess

	// global relabeling parameters
	int workSinceLastGR; // amount of work done in an active set since last global relabeling
	const float freq = 0.5; // hi_pr GR param 
	const int alpha = 6; // hi_pr GR param 
	const int beta = 12; // hi_pr GR param 

	// init the flow network
	// read the flow instance, construct a graph using the augmented vertices and edges
	// and initialize source excess to be inf
	void init(FlowInstance fi);
	
	// do a global relabel: run a reverse BFS from sink
	// to get exact distance labels for all vertices
	// the BFS only uses edges that are unsatisfied, and only relabels vertices 
	// which do not have label set to n
	void global_relabel(); 

	// push flow out of a vertex u
	// tries to push flow out of all admissable edges out of u
	// if we push to a vertex v which results in positive excess, then 
	// we add v to the next active set of vertices 
	// if we pull out of all edges and we still have positive excess, 
	// then we need to add u to active again and repeat next round
	// this function fills in the need_relabel variable to indicate if u needs to be relabeled
	void push(vertex_id ui, bool& need_relabel);
	
	// try pushing a vertex v to the argument sequence
	// use this push to a local vertex queue of active pushes that are made in the next round
	void push_active(parlay::sequence<vertex_id> &a, vertex_id vi);

	// relabel a vertex by finding the minimum label of neighbor vertices, 
	// and set to min_label + 1, or n if this vertex is done pushing
	void relabel(vertex_id ui); 
	
	// construct a flow solver
	// calls init
	FIFOSyncParPR(FlowInstance fi);
	
	// from the push_relabel example on parlaylib
	// check the correctness of the flow according to the correctness proof of push relabel 
	// - check that flow matches excess at each vertex
	// - no flow exceeds capacity
	// - no invalid labeling between vertices
	// - no left over excess
	void check_correctness();

	public: 
		// solve a flow instance by initializing, and run an initial global relabeling 
		// afterwards, run rounds of doing a push then relabel on all admissable edges out 
		// of the active set, then compute the next set of active vertices 
		// if we cross a work threshold, do another global relabeling 
		// terminate when there are no more vertices in the active set
		static int solve(FlowInstance fi);
};

#endif
