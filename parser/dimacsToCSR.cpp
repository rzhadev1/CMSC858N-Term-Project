#include <iostream>
#include <string>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/io.h>
#include <assert.h>
#include "IO.h"
using namespace std;

using vertex = int;
using edge = tuple<vertex, float>; // an edge is (dst vertex, weight/capacity)
using nested_seq = parlay::sequence<parlay::sequence<edge>>;
using AdjList = nested_seq; 

struct DimacsFlowAdjList {
	int n; 
	int m; 
	vertex source; 
	vertex sink;
	AdjList adjacency_list;
};

DimacsFlowAdjList readDimacs(const string& filename) { 
	auto str = parlay::file_map(filename);
	auto lines = parlay::tokens(str, [] (char c) {return c == '\n';});  // split the line on newlines into sequence of strings
	
	int S = -1, T = -1; // the source and sink 
	int n = 0, m = 0;  // number of vertices, number of edges
	
	AdjList dimacs_adj;
	// parse all the comments and problem statement line from dimacs
	
	for(const auto& line : lines) {
		char type = line[0];
		auto tokens = parlay::tokens(line, [] (char c) {return c == ' ';});

		if(type == 'c') { 
			cout << line << endl;  // comment line
		}

		else if (type == 'p') {  // problem statement line contains n,m
			n = parlay::chars_to_int(tokens[2]);
			m = parlay::chars_to_int(tokens[3]);
			assert(n > 0); 
			assert(m > 0);
			
			cout << "num vertices: " << n << endl; 
			cout << "num edges: " << m << endl; 
			
			// for each vertex, create an empty sequence
			dimacs_adj = parlay::tabulate<parlay::sequence<edge>>(n, [&] (size_t i) {return parlay::sequence<edge>();});
		}

		else if(type == 'n') { // specify source/sink
			if (tokens[2][0] == 's') {
				S = parlay::chars_to_int(tokens[1]);
				cout << "source: " << S << endl;
			}
			else if (tokens[2][0] == 't') {
				T = parlay::chars_to_int(tokens[1]);
				cout << "sink: " << T << endl; 
			}

			else {
				cout << "invalid node descriptor line" << endl;
			}
		}

		else if(type == 'a') {
			
			// we must have sequences initialized to get here
			assert(n > 0); 
			assert(m > 0);
			
			// note that vertices are 1,..,n
			vertex src = parlay::chars_to_int(tokens[1]) - 1;
			vertex dst = parlay::chars_to_int(tokens[2]) - 1;

			float cap = static_cast<float>(parlay::chars_to_int(tokens[3]));
			
			dimacs_adj[src].push_back(edge{dst, cap}); 
		}

		else {
			cout << "invalid line!" << endl; 
		}
		
	}

	DimacsFlowAdjList graph;
	graph.n = n; 
	graph.m = m; 
	graph.source = S; 
	graph.sink = T; 
	graph.adjacency_list = dimacs_adj;
	return graph;
}

// convert adj list to compressed sparse row format, and write to output file path
// pbbs uses these as format
int writeDimacsFlowToCSR(DimacsFlowAdjList graph, const string& outfp) {
	AdjList adj = graph.adjacency_list;
	int n = graph.n; 
	int m = graph.m; 

	// get vertex out degrees, scan to get prefix list
	auto outdeg_scan = parlay::scan(parlay::map(adj, [&] (const auto& edge_seq) {return edge_seq.size();}));
	auto offset = get<0>(outdeg_scan); // get the outdegree prefix sum
	offset.insert(offset.begin(), 0); // offsets is size n+1; first element should be 0, b/c we encode differences
	
	assert(m == get<1>(outdeg_scan)); // total number of edges should be m
	assert(offset.size() == n+1); // offsets must be size n+1
	
	// extract edges
	// get dst vertex sequence, flatten to sequence, then flatten to a single contiguous sequence
	auto dst_edges = parlay::flatten(parlay::map(adj, [&] (const auto& edge_seq) {
		auto dst_seq = parlay::map(edge_seq, [&] (const auto& edge) {return get<0>(edge);});
		return dst_seq;
	}));
	
	// extract capacities, similar
	auto cap_edges = parlay::flatten(parlay::map(adj, [] (auto& edge_seq) {
		auto cap_seq = parlay::map(edge_seq, [&] (const auto& edge) {return get<1>(edge);});
		return cap_seq;
	}));

	// all edges collected 
	assert(dst_edges.size() == m);
	assert(cap_edges.size() == m);
	
	parlay::sequence<int> out1 = parlay::tabulate<int>(2 + n + m, [&] (int i) {return i;});
	parlay::sequence<int> out2 = parlay::tabulate<int>(m, [&] (int i) {return i;});
	out1[0] = n; 
	out2[1] = m; 
	
	// write offsets to [2, 2+n)
	parlay::parallel_for(0, n, [&] (size_t i) {
		out1[i+2] = offset[i];
	});
	
	// write out edges to out1[2+n, 2+n+m) 
	// write capacities to out2[0,...,m)
	parlay::parallel_for(0, n, [&] (size_t i) {
		size_t o = offset[i];
		auto v_edges = adj[i];

		for (int j = 0; j < v_edges.size(); j++) { 
			out1[2 + n + o +j] = get<0>(v_edges[j]);
			out2[o + j] = get<1>(v_edges[j]);
		}
	});

	int r = benchIO::write2SeqToFile("WeightedAdjGraph", out1, out2, outfp.c_str());
	return r; 
}

int main() { 
	string fp = "/Users/richardz/Desktop/CMSC858N/final/term_project/parser/data/dimacs/BL06-camel-sml/BL06-camel-sml.max";
	string outfp = "/Users/richardz/Desktop/CMSC858N/final/term_project/parser/data/dimacs/BL06-camel-sml/BL06-camel-sml.adj";
	DimacsFlowAdjList dimacs = readDimacs(fp);
	int r = writeDimacsFlowToCSR(dimacs, outfp);
	return 0; 
}
