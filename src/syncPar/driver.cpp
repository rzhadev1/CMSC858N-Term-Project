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

	auto usage = "Usage: syncPar <filename>";
	
	if(argc != 2) std::cout << usage << std::endl;
	else {
		std::string graph_file = argv[1]; 
		FlowInstance flow_problem = readDimacsToFlowInstance(graph_file);
		
		for (int i = 0; i < 3; i++) {
			std::cout << "iteration: " << i << std::endl;
			int result = FIFOSyncParPR::solve(flow_problem);
		}
	}
	return 0;
}
