#include <iostream>
#include <string>
#include <map>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/internal/get_time.h>
#include <parlay/io.h>

#include "parser.h"
#include "hash_map.h"

// Read the dimacs file to a parlay adjacency list representation
FlowInstance readDimacsToFlowInstance(const std::string& filename) {
		parlay::internal::timer timer("Time");
    auto str = parlay::file_map(filename);
		timer.next("File map"); 

    auto lines = parlay::tokens(str, [&] (char c) {return c == '\n';});  // split the line on newlines into sequence of std::strings
    timer.next("Tokens");

    vertex_id S = -1, T = -1; // the source and sink 
    int n = 0, m = 0;  // number of vertices, number of edges
    
   	using edge_key = std::pair<vertex_id, vertex_id>; // map the (u,v) edge to its capacity
		// get all lines without type 'a', meaning either 
		// comment, problem statement, or source/sink lines and set values
		// accordingly
		auto non_edge_lines = parlay::map(parlay::filter(lines, [&] (auto& line) {
			return line[0] != 'a';
		}), [&] (auto& non_edge_line) {

			char type = non_edge_line[0];
			auto tokens = parlay::tokens(non_edge_line, [] (char c) {return c == ' ';});
			if(type == 'n') {
				if(tokens[2][0] == 's') {
					S = parlay::chars_to_int(tokens[1]) - 1; 
				}
				else if(tokens[2][0] == 't') {
					T = parlay::chars_to_int(tokens[1]) - 1;
				}
			}

			else if(type == 'p') {
				n = parlay::chars_to_int(tokens[2]); 
				m = parlay::chars_to_int(tokens[3]);
			}

			return non_edge_line;
	
		});
		
		timer.next("non edge lines");
		/*
    // parse all the comments and problem statement line from dimacs
    for(const auto& line : lines) {
        char type = line[0];
        auto tokens = parlay::tokens(line, [] (char c) {return c == ' ';});

        if(type == 'c') { 
        }

        else if (type == 'p') {  // problem statement line contains n,m
            n = parlay::chars_to_int(tokens[2]);
            m = parlay::chars_to_int(tokens[3]);
            
            // for each vertex, create an empty sequence of edges
            dimacs_adj = parlay::tabulate<edges>(n, [&] (size_t i) {return edges();});				
						// only need m entries at most
						edge_map = hash_map<edge_key, weight>(m);
        }

        else if(type == 'n') { // specify source/sink
            if (tokens[2][0] == 's') {
                S = parlay::chars_to_int(tokens[1]) - 1;
            }
            else if (tokens[2][0] == 't') {
                T = parlay::chars_to_int(tokens[1]) - 1;
            }

        }
		}
		*/

		// find all lines with 'a' for edges
		auto edge_lines = parlay::filter(lines, [&] (auto& line) {
			return line[0] == 'a';
		});
		
		timer.next("edge lines");
	
		weighted_graph dimacs_adj = parlay::tabulate<edges>(n, [&] (size_t i) {
			return edges();
		});

		hash_map<edge_key, weight> edge_map(m);

		// in parallel, insert into the phase concurrent hash table
		parlay::parallel_for(0, edge_lines.size(), [&] (long i) { 
			auto tokens = parlay::tokens(edge_lines[i], [] (char c) {return c == ' ';});
			vertex_id src = parlay::chars_to_int(tokens[1]) - 1; 
			vertex_id dst = parlay::chars_to_int(tokens[2]) - 1;
			weight cap = static_cast<weight>(parlay::chars_to_int(tokens[3]));

			edge_map.insert({src, dst}, cap);
		});

		timer.next("insert edges");
		/*
		for(auto& line : edge_lines) { 
        if(type == 'a') {
            // we must have sequences initialized to get here
            
            vertex_id src = parlay::chars_to_int(tokens[1]) - 1;
            vertex_id dst = parlay::chars_to_int(tokens[2]) - 1;

            weight cap = static_cast<weight>(parlay::chars_to_int(tokens[3]));

            // this line specifies a (u,v) edge. we only update capacities for 
            // (u,v) edges. all reverse edges are assumed to have 0 capacity until 
            // we see the reverse edge as (u,v)
						//
            // is edge (u,v) not in the graph?
            if(edge_map.find({src, dst}) == edge_map.end()) {
                edge_map.insert({{src, dst}, cap}); // insert into edge map
            }

            // in the graph, update its capacity
            else {
                edge_map.at({src, dst}) = cap;
            }

            // is edge (v,u) not in the graph?
            if(edge_map.find({dst, src}) == edge_map.end()) { 
                edge_map.insert({{dst, src}, 0}); // insert into edge map with 0 capacity
            }
        }

    }
		*/
		
		timer.next("parse");
		/*
    for(auto& edge : edge_map) {
        vertex_id src = std::get<0>(edge.first); 
        vertex_id dst = std::get<1>(edge.first);
        weight cap = edge.second;
        
        dimacs_adj[src].push_back({dst, cap});
    }
		*/
		timer.next("parlay seq conversion");

    FlowInstance flow;
    flow.n = n; 
    flow.m = m; 
    flow.source = S; 
    flow.sink = T; 
    flow.wgh_graph = dimacs_adj;
    return flow;
}
