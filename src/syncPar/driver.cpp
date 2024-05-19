#include <iostream>
#include <string>
#include <fstream>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/internal/get_time.h>

#include "parser.h"
#include "flow.h"
//#include "example_flow.h"


// **************************************************************
// Driver
// **************************************************************

int main(int argc, char* argv[]) {

	auto usage = "Usage: syncPar <filename>";
	
	if(argc != 2) std::cout << usage << std::endl;
	else {
		std::string graph_file = argv[1]; 
		FlowInstance flow_problem = readDimacsToFlowInstance(graph_file);
		int result = FIFOSyncParPR::solve(flow_problem);
		
		/*
		int n = flow_problem.n;
		int result;	
		parlay::internal::timer t("Time");
		for (int i=0; i < 2; i++) {
			result = maximum_flow(flow_problem.wgh_graph, flow_problem.source, flow_problem.sink);
			t.next("push_relabel_max_flow");
		}
		*/
		std::cout << "max flow: " << result << std::endl;
	}
	return 0;
}
