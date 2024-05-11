#include <assert.h>
#include <iostream>
#include <string>
#include <map>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/io.h>

#include "graph_utils.h"


namespace Parser {
	using vertex_id = int;
	using weight = int;
	using edges = parlay::sequence<std::pair<vertex_id,weight>>;
	using weighted_graph = parlay::sequence<edges>;
	using graph = parlay::sequence<parlay::sequence<vertex_id>>;
	using utils = graph_utils<vertex_id>;

	struct FlowInstance {
		int n; 
		int m; 
		vertex_id source; 
		vertex_id sink;
		weighted_graph adj_list;
		//std::map<std::pair<vertex_id, vertex_id>, weight> edge_map;
	};

	// Read the dimacs file to a parlay adjacency list representation
	FlowInstance readDimacsToFlowInstance(const std::string& filename) { 
		auto str = parlay::file_map(filename);
		auto lines = parlay::tokens(str, [] (char c) {return c == '\n';});  // split the line on newlines into sequence of std::strings
		
		vertex_id S = -1, T = -1; // the source and sink 
		int n = 0, m = 0;  // number of vertices, number of edges
		
		weighted_graph dimacs_adj;

		// compare operator for edges (which are represented as pair<vertex, weight>)
		// in the edge_map
		/*
		auto compare_edge = [](const std::pair<vertex_id, weight> edge1, std::pair<vertex_id, weight> edge2) {
			auto edge1_src = std::get<0>(edge1);
			auto edge1_dst = std::get<1>(edge1); 

			auto edge2_src = std::get<0>(edge2); 
			auto edge2_dst = std::get<1>(edge2); 

			return (edge1_src < edge2_src) && (edge1_dst < edge2_dst);
		};
		std::map<std::pair<vertex_id, vertex_id>, weight, decltype(compare_edge)> edge_map(compare_edge); 
		*/
		std::map<std::pair<vertex_id, vertex_id>, weight> edge_map; 

		// parse all the comments and problem statement line from dimacs
		for(const auto& line : lines) {
			char type = line[0];
			auto tokens = parlay::tokens(line, [] (char c) {return c == ' ';});

			if(type == 'c') { 
				std::cout << line << std::endl;  // comment line
			}

			else if (type == 'p') {  // problem statement line contains n,m
				n = parlay::chars_to_int(tokens[2]);
				m = parlay::chars_to_int(tokens[3]);
				assert(n > 0); 
				assert(m > 0);
				
				std::cout << "num vertices: " << n << std::endl; 
				std::cout << "num edges: " << m << std::endl; 
				
				// for each vertex, create an empty sequence of edges
				dimacs_adj = parlay::tabulate<edges>(n, [&] (size_t i) {return edges();});
			}

			else if(type == 'n') { // specify source/sink
				if (tokens[2][0] == 's') {
					S = parlay::chars_to_int(tokens[1]) - 1;
					std::cout << "source: " << S << std::endl;
				}
				else if (tokens[2][0] == 't') {
					T = parlay::chars_to_int(tokens[1]) - 1;
					std::cout << "sink: " << T << std::endl; 
				}

				else {
					std::cout << "invalid node descriptor line" << std::endl;
				}
			}

			else if(type == 'a') {
				
				// we must have sequences initialized to get here
				assert(n > 0); 
				assert(m > 0);
				
				vertex_id src = parlay::chars_to_int(tokens[1]) - 1;
				vertex_id dst = parlay::chars_to_int(tokens[2]) - 1;

				weight cap = static_cast<weight>(parlay::chars_to_int(tokens[3]));

				// this line specifies a (u,v) edge. we only update capacities for 
				// (u,v) edges. all reverse edges are assumed to have 0 capacity until 
				// we see the reverse edge as (u,v)

				// is edge (u,v) not in the graph?
				if(edge_map.find({src, dst}) == edge_map.end()) {
					// dimacs_adj[src].push_back({dst, cap}); // add to the graph
					edge_map.insert({{src, dst}, cap}); // insert into edge map
				}

				// in the graph, update its capacity
				else {
					edge_map.at({src, dst}) = cap;
				}

				// is edge (v,u) not in the graph?
				if(edge_map.find({dst, src}) == edge_map.end()) { 
					// dimacs_adj[dst].push_back({src, 0});
					edge_map.insert({{dst, src}, 0}); // insert into edge map with 0 capacity
				}

				// if edge in either direction already exists, update its capacity to the value in this line
				// dimacs_adj[src].push_back({dst, cap}); 
			}

			else {
				std::cout << line << std::endl;
				std::cout << "invalid line!" << std::endl; 
			}
			
		}

		// read edge map and construct weighted graph

		for(auto& edge : edge_map) {
			vertex_id src = std::get<0>(edge.first); 
			vertex_id dst = std::get<1>(edge.first);
			weight cap = edge.second;
			
			dimacs_adj[src].push_back({dst, cap});
			//std::cout << std::get<0>(edge.first) << " " << std::get<1>(edge.first) << " " << edge.second << std::endl;	
		}



		FlowInstance flow;
		flow.n = n; 
		flow.m = m; 
		flow.source = S; 
		flow.sink = T; 
		flow.adj_list = dimacs_adj;
		return flow;
	}
	/*
	FlowInstance symmetrize(const FlowInstance df) {

		weighted_graph dimacs_adj = df.adj_list;
		auto edge_map = df.edge_map;

		int n = df.n; 
		int m = df.m;
		
		// Parlay example requires symmetric graph? 
		for(vertex_id src = 0; src < n; src++) {
			for(auto& edge : dimacs_adj[src]) { 

				vertex_id dst = std::get<0>(edge); 
				weight cap = std::get<1>(edge);
				
				// reverse edge is not in the weighted graph, so add with 0 capacity
				if(edge_map.find({dst, src}) == edge_map.end()) {
					dimacs_adj[dst].push_back({src, 0}); 
				}

			}
		}

		FlowInstance flow;
		flow.n = n; 
		flow.m = m; // do we need to add the number of edges we added from symmetrizing?
		flow.source = df.source; 
		flow.sink = df.sink; 
		flow.adj_list = dimacs_adj;
		flow.edge_map = edge_map;

		return flow;

	}
	*/
}


