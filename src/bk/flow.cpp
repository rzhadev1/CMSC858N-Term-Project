#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <boost/config.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/graph/read_dimacs.hpp>
#include <boost/graph/graph_utility.hpp>

#include <parlay/internal/get_time.h>


int main(int argc, char* argv[])
{
  using namespace boost;


	auto usage = "Usage: syncPar <filename>";

	if(argc != 2) std::cout << usage << std::endl;
	else {
		std::string graph_file = argv[1]; 
		std::ifstream in(graph_file);

		typedef adjacency_list_traits < vecS, vecS, directedS > Traits;
		typedef adjacency_list < vecS, vecS, directedS,
			property < vertex_name_t, std::string,
			property < vertex_index_t, long,
			property < vertex_color_t, boost::default_color_type,
			property < vertex_distance_t, long,
			property < vertex_predecessor_t, Traits::edge_descriptor > > > > >,

			property < edge_capacity_t, long,
			property < edge_residual_capacity_t, long,
			property < edge_reverse_t, Traits::edge_descriptor > > > > Graph;

		Graph g;
		property_map < Graph, edge_capacity_t >::type
				capacity = get(edge_capacity, g);
		property_map < Graph, edge_residual_capacity_t >::type
				residual_capacity = get(edge_residual_capacity, g);
		property_map < Graph, edge_reverse_t >::type rev = get(edge_reverse, g);
		Traits::vertex_descriptor s, t;
		read_dimacs_max_flow(g, capacity, rev, s, t, in);

		std::vector<default_color_type> color(num_vertices(g));
		std::vector<long> distance(num_vertices(g));
		
		parlay::internal::timer timer("Time");
		long flow = boykov_kolmogorov_max_flow(g ,s, t);
		timer.next("bk time"); 

		std::cout << "c  The total flow:" << std::endl;
		std::cout << "s " << flow << std::endl << std::endl;
	}
	return EXIT_SUCCESS;
}
