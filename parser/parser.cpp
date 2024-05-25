#include <iostream>
#include <string>
#include <map>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/io.h>
#include "parser.h"

FlowInstance readDimacsToFlowInstance(const std::string& filename) {

		parlay::internal::timer timer("time");
    auto str = parlay::file_map(filename);
    timer.next("file map"); 
		auto lines = parlay::tokens(str, [&] (char c) {return c == '\n';});  // split the line on newlines into sequence of std::strings
    timer.next("tokens");
    vertex_id S = -1, T = -1; // the source and sink 
    int n = 0, m = 0;  // number of vertices, number of edges
    
    weighted_graph dimacs_adj;

    std::map<edge, weight> edge_map; 

    for(const auto& pline : lines) {
				const char* line = pline.data();
        char type = line[0];
				
				// slow!
        // auto tokens = parlay::tokens(line, [] (char c) {return c == ' ';});
        if(type == 'c') {}

        else if (type == 'p') {  // problem statement line contains n,m
					std::sscanf(line, "p %*3s %d %d", &n, &m);	
        }

        else if(type == 'n') { // specify source/sink
					int id; 
					char which;
        	std::sscanf(line, "n %d %c", &id, &which);

					if(which == 's') {
						S = id; 
					}
					else if(which == 't') {
						T = id;
					}
				}

        else if(type == 'a') {
					vertex_id src; 
					vertex_id dst; 
					weight cap;
					std::sscanf(line, "a %d %d %d", &src, &dst, &cap);

					// 1 indexed -> 0 indexed
					src--; 
					dst--;

					auto e1 = edge_map.insert({{src, dst}, cap});
					auto e2 = edge_map.insert({{dst, src}, 0});
					
					// if the edge already exists, 
					// then set the capacity instead
					if(e1.second == false) {
						e1.first -> second = cap;
					}
				}

        else {
            std::cout << "invalid line!: " << line << std::endl; 
        }
    }

		timer.next("parse"); 
		dimacs_adj = parlay::tabulate<edges>(n, [&] (size_t i) {return edges();});
    for(auto& edge : edge_map) {
        vertex_id src = std::get<0>(edge.first); 
        vertex_id dst = std::get<1>(edge.first);
				weight cap = edge.second;
				
        dimacs_adj[src].push_back({dst, cap});
    }
		timer.next("convert graph");

    FlowInstance flow;
    flow.n = n; 
    flow.m = m; 
    flow.source = S; 
    flow.sink = T; 
    flow.wgh_graph = dimacs_adj;
    return flow;
}
