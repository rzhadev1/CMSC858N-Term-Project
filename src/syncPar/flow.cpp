#include <iostream>
#include <string>
#include <fstream>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/internal/get_time.h>

#include "flow.h"
#include "graphIO.h"
#include "graph_utils.h"

// **************************************************************
// Driver
// **************************************************************

int main(int argc, char* argv[]) {
	using vertex = int;
	using utils = graph_utils<vertex_id>;
	using vertices = parlay::sequence<vertex>;

	char *graph_file = "/Users/richardz/Desktop/CMSC858N/final/term_project/parser/data/dimacs/BL06-camel-sml/BL06-camel-sml.adj";
	
	ifstream graph(graph_file);

	auto flowgraph = readFlowGraphDimacs(graph);

	//auto graph<vertices, weight, edges> = benchIO::readWghGraphFromFile(graph_file);
	
	/*
	int n = graph.size();
	int result;
	//parlay::internal::timer t("Time");
	for (int i=0; i < 2; i++) {
		result = maximum_flow(graph, 0, 1);
		//t.next("push_relabel_max_flow");
	}


	std::cout << "max flow: " << result << std::endl;
	*/
	return 0;
}
