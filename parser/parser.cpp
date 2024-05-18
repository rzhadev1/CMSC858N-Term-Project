#include <iostream>
#include <string>
#include <map>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/io.h>
#include "parser.h"

// Read the dimacs file to a parlay adjacency list representation
FlowInstance readDimacsToFlowInstance(const std::string& filename) {
    auto str = parlay::file_map(filename);
    auto lines = parlay::tokens(str, [&] (char c) {return c == '\n';});  // split the line on newlines into sequence of std::strings
    
    vertex_id S = -1, T = -1; // the source and sink 
    int n = 0, m = 0;  // number of vertices, number of edges
    
    weighted_graph dimacs_adj;
    
    using edge_key = std::pair<vertex_id, vertex_id>; // map the (u,v) edge to its capacity
    std::map<edge_key, weight> edge_map; 

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
            
            vertex_id src = parlay::chars_to_int(tokens[1]) - 1;
            vertex_id dst = parlay::chars_to_int(tokens[2]) - 1;

            weight cap = static_cast<weight>(parlay::chars_to_int(tokens[3]));

            // this line specifies a (u,v) edge. we only update capacities for 
            // (u,v) edges. all reverse edges are assumed to have 0 capacity until 
            // we see the reverse edge as (u,v)

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

        else {
            std::cout << "invalid line!: " << line << std::endl; 
            exit(1);
        }
    }

    // read edge map and construct weighted graph
    //std::cout << "edge map: " << std::endl;
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