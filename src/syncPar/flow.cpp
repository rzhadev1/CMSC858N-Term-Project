#include <iostream>
#include <string>
#include <fstream>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/internal/get_time.h>
#include "parser.h"
#include "flow.h"

// **************************************************************
// Driver
// **************************************************************

int main(int argc, char* argv[]) {
	std::string graph_file = "/Users/richardz/Desktop/CMSC858N/final/term_project/data/dimacs/BL06-camel-sml/BL06-camel-sml.max";
	// std::string graph_file = "/Users/richardz/Desktop/CMSC858N/final/term_project/data/dimacs/example.max";
	//std::string graph_file = "/Users/richardz/Desktop/CMSC858N/final/term_project/data/dimacs/BVZ-tsukuba/BVZ-tsukuba2.max";
	Parser::FlowInstance graph = Parser::readDimacsToFlowInstance(graph_file);
	//Parser::FlowInstance symm_graph = Parser::symmetrize(graph);
	
	int n = graph.n;
	int result;
	parlay::internal::timer t("Time");
	for (int i=0; i < 2; i++) {
		result = maximum_flow(graph.adj_list, graph.source, graph.sink);
		t.next("push_relabel_max_flow");
	}

	std::cout << "max flow: " << result << std::endl;
	return 0;
}
