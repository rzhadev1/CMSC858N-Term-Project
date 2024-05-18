#include "flow.h"


// TODO: 
// - switch to something sequential when active set is small?
// - probably should get off of using map in the parser, really slow on these big graphs


void FIFOSyncParPR::init(FlowInstance fi) { 
	n = fi.n; 
	m = parlay::reduce(parlay::map(fi.wgh_graph, parlay::size_of{}));
	sink = fi.sink; 
	source = fi.source;

	vertices = parlay::sequence<Vertex>(n);
	
	// create all vertices and edge sequences
	parlay::parallel_for(0, n, [&] (vertex_id u) {
		vertices[u].edges = parlay::tabulate<Edge>(fi.wgh_graph[u].size(), [&] (int j) {
			vertex_id v = fi.wgh_graph[u][j].first; 
			int wgh = fi.wgh_graph[u][j].second;
			return Edge{wgh, 0, nullptr, u, v};
		});
	});
	
	// a cool trick from the parlaylib push relabel max flow example:
	// https://github.com/cmuparlay/parlaylib/blob/master/examples/push_relabel_max_flow.h 
	// collect the edges into list of pairs (e=(u,v), e_f=(v,u))
	auto x = parlay::flatten(parlay::tabulate(n, [&] (vertex_id u) {
		return parlay::delayed_map(vertices[u].edges, [&, u] (Edge& e) {
			auto p = std::pair{std::min(u,e.v), std::max(u,e.v)};
			return std::pair{p, &e};});}));

	// sort the edges by their u vertex
	auto y = parlay::sort(std::move(x), [&] (auto a, auto b) {return a.first < b.first;});

	// cross link using the fact that now (u,v) and (v,u) are adjacent with 
	// each other in this array
	// note that we should only do this for m/2 edges, because it would be symmetric for (m/2, m]
	parlay::parallel_for(0, m/2, [&] (long i) {
		y[2*i].second->res_edge = y[2*i+1].second;
		y[2*i].second->res_edge->capacity = y[2*i+1].second->capacity;
		y[2*i+1].second->res_edge = y[2*i].second;
		y[2*i+1].second->res_edge->capacity = y[2*i].second->capacity;});

	vertices[source].excess = std::numeric_limits<int>::max();	
	vertices[source].label = n;
}


FIFOSyncParPR::FIFOSyncParPR(FlowInstance fi) {
	init(fi);
}

void FIFOSyncParPR::relabel() {
}

void FIFOSyncParPR::global_relabel() {
}

void FIFOSyncParPR::push() {
}

int FIFOSyncParPR::solve(FlowInstance fi) {
	FIFOSyncParPR flow = FIFOSyncParPR(fi);
	
	Vertex& s = flow.vertices[flow.source];
	
	// saturate all source adjacent edges
	parlay::parallel_for(0, s.edges.size(), [&] (int j) {
		Edge& e = s.edges[j];
		e.flow = e.capacity;
		(e.res_edge) -> flow = -e.capacity;
		flow.vertices[e.v].excess = e.capacity;
	});

	return 0;
}
