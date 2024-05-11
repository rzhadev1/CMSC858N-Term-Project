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

int main()
{
  using namespace boost;

  std::string graph_file = "/Users/richardz/Desktop/CMSC858N/final/term_project/data/dimacs/BL06-camel-sml/BL06-camel-sml.max";
  //std::string graph_file = "/Users/richardz/Desktop/CMSC858N/final/term_project/data/dimacs/example.max";
  //std::string graph_file = "/Users/richardz/Desktop/CMSC858N/final/term_project/data/dimacs/BVZ-tsukuba/BVZ-tsukuba0.max";
  //std::string graph_file = "/Users/richardz/Desktop/CMSC858N/final/term_project/data/dimacs/example.max";
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
  long flow = boykov_kolmogorov_max_flow(g ,s, t);

  std::cout << "c  The total flow:" << std::endl;
  std::cout << "s " << flow << std::endl << std::endl;

  /*
  std::cout << "c flow values:" << std::endl;
  graph_traits < Graph >::vertex_iterator u_iter, u_end;
  graph_traits < Graph >::out_edge_iterator ei, e_end;

  
  for (boost::tie(u_iter, u_end) = vertices(g); u_iter != u_end; ++u_iter)
    for (boost::tie(ei, e_end) = out_edges(*u_iter, g); ei != e_end; ++ei)
      if (capacity[*ei] > 0)
        std::cout << "f " << *u_iter << " " << target(*ei, g) << " "
          << (capacity[*ei] - residual_capacity[*ei]) << std::endl;
  */
  return EXIT_SUCCESS;
}
