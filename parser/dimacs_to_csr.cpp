#include <iostream>
#include <string>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/io.h>
#include <assert.h>

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
void writeDimacsFlowToCSR(DimacsFlowAdjList graph, const string& outfp) {
	AdjList adj = graph.adjacency_list;
	int n = graph.n; 
	int m = graph.m; 

	parlay::sequence<int> offset = parlay::tabulate<int>(n+1, [&] (int i) {return -1;});
	parlay::sequence<int> edges = parlay::tabulate<int>(m, [&] (int i) {return -1;};);
	parlay::sequence<float> cap = parlay::tabulate<float>(m, [&] (int i) {return 0;});
	

	
}

int main() { 
	string fp = "/Users/richardz/Desktop/CMSC858N/final/term_project/parser/data/dimacs/BL06-camel-sml/BL06-camel-sml.max";
	DimacsFlowAdjList dimacs = readDimacs(fp);

	return 0; 
}
