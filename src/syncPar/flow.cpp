#include <iostream>
#include <string>
#include <fstream>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/internal/get_time.h>

#include "parser.h"
#include "flow.h"

using vertex_id = int;
using weight = int;
using edges = parlay::sequence<std::pair<vertex_id,weight>>;
using weighted_graph = parlay::sequence<edges>;

// **************************************************************
// Driver
// **************************************************************

int main(int argc, char* argv[]) {

	auto usage = "Usage: syncPar <filename>";

	if(argc != 2) std::cout << usage << std::endl;
	else {
		std::string graph_file = argv[1]; 
		FlowInstance graph = readDimacsToFlowInstance(graph_file);
		
		int n = graph.n;
		int result;	
		parlay::internal::timer t("Time");
		for (int i=0; i < 2; i++) {
			result = maximum_flow(graph.adj_list, graph.source, graph.sink);
			t.next("push_relabel_max_flow");
		}

		std::cout << "max flow: " << result << std::endl;
	}
	return 0;
}