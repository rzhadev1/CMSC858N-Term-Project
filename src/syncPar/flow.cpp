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
			return Edge{wgh, 0, wgh, nullptr, u, v};
		});

		vertices[u].outDegree = vertices[u].edges.size(); 
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
		y[2*i].second->res_capacity = y[2*i+1].second->capacity;
		y[2*i+1].second->res_edge = y[2*i].second;
		y[2*i+1].second->res_capacity = y[2*i].second->capacity;});

	vertices[source].excess = std::numeric_limits<int>::max();	
}


FIFOSyncParPR::FIFOSyncParPR(FlowInstance fi) {
	init(fi);
}

void FIFOSyncParPR::push(vertex_id ui, bool& need_relabel) {

	// this comes straight from the paper
	Vertex& u = vertices[ui];
	while (u.excess > 0 && u.current < u.edges.size()) {
		Edge& e = u.edges[u.current];
		Vertex& v = vertices[e.v];

		// check if the current edge is admissable for a push
		if (e.flow < e.capacity && u.label > v.label) {

			int delta = std::min(e.capacity - e.flow, u.excess);
			if (delta == e.capacity - e.flow) u.current++; // saturating
			e.flow += delta;
			e.res_edge->flow -= delta;
			u.excess -= delta;
			v.addedExcess += delta; // atomic
			
			push_active(u.discoveredVertices, e.v);
		} 

		else { 
			u.current++;
		}
	}

	// u is still active, but needs to be relabeled
	if (u.excess > 0) push_active(u.discoveredVertices, ui);
	
	need_relabel = u.current == u.edges.size(); 
}

void FIFOSyncParPR::push_active(parlay::sequence<vertex_id>& a, vertex_id vi) {
	Vertex& v = vertices[vi]; 
	bool flag = false; 
	if(!v.isDiscovered && v.isDiscovered.compare_exchange_strong(flag, true))
		a.push_back(vi);
}

void FIFOSyncParPR::relabel(vertex_id ui) {
	Vertex& u = vertices[ui]; 
	u.current = 0;

	// get minimum neighbor
	int min_next_label = parlay::reduce(parlay::delayed_map(u.edges, [&] (Edge& e) {
		
		// if edge isn't saturated, then return this new label
		if(e.flow < e.capacity) {
			return vertices[e.v].label; 
		}

		// edge is saturated
		return n;
	}), parlay::minimum<int>());

	u.newlabel = std::min(n, min_next_label + 1); // note: we use the copy here
} 

void FIFOSyncParPR::global_relabel() {
	// start from sink, and do a reverse bfs to update labels
	auto distances = parlay::tabulate<std::atomic<vertex_id>>(n, [&] (long ui) {
		return n;
	});

	parlay::sequence<vertex_id> frontier; // queue, init with just sink
	frontier.push_back(sink);

	distances[sink] = 0;
	vertex_id level = 0;
	
	while (frontier.size() > 0) {
		level++; 
    // get edges out of the current frontier
		auto out_edges = parlay::flatten(parlay::map(frontier, [&] (vertex_id u) {
			return vertices[u].edges;
		}));
		
		// get unsaturated edges (residual capacity is > 0), and set labels for their 
		// vertices if they are unset using CAS
		// get next frontier
		frontier = parlay::map(parlay::filter(out_edges, [&] (Edge& e) {
			vertex_id expected = n; 
			vertex_id v = e.v;
			
			// is this edge satisfied?
			bool sat = e.res_capacity == -e.flow;
			bool cond = distances[v] == n;

			return (cond) && !sat && distances[v].compare_exchange_strong(expected, level);
		}), 
		[&] (Edge& e) {
				return e.v;
		});
	}

	// set labels
	parlay::parallel_for(0, n, [&] (vertex_id u) {
		vertices[u].label = distances[u];
		vertices[u].newlabel = distances[u]; 
		vertices[u].current = 0;
	}); 
	
	// refresh the vertices in the active set
	active = parlay::filter(parlay::iota<vertex_id>(n), [&] (vertex_id vi) {
		Vertex& v = vertices[vi];
		return v.label != 0 && v.label < n && v.excess > 0;
	});

	workSinceLastGR = 0;
}

int FIFOSyncParPR::solve(FlowInstance fi) {
	FIFOSyncParPR flow = FIFOSyncParPR(fi);

	flow.global_relabel(); // get starting labels
	flow.workSinceLastGR = 0;

	int iters = 0;
	while (flow.active.size() > 0) { 

		//std::cout << flow.active.size() << std::endl;
		//std::cout << flow.vertices[flow.sink].excess << std::endl;
		// try to push on all active vertices, relabel if needed
		parlay::for_each(flow.active, [&] (vertex_id ui) {
			Vertex& u = flow.vertices[ui];
			u.work = 0;

			// core: push and relabel this active vertex
			bool need_relabel = false;
			flow.push(ui, need_relabel);
			if(u.label < flow.n && u.label > 0 && need_relabel) {
				flow.relabel(ui);
			}

			// update work at this vertex
			u.work = u.outDegree + flow.beta;
		});
		
		// update variables and get next set of active vertices
		flow.active = parlay::flatten(parlay::map(flow.active, [&] (vertex_id ui) {
			Vertex& u = flow.vertices[ui];
			u.label = u.newlabel;
			for(vertex_id vi : u.discoveredVertices) {
				Vertex& v = flow.vertices[vi]; 
				v.excess += v.addedExcess; // update excess from previous round
				v.addedExcess = 0; 	// reset 
				v.isDiscovered = false;
			}
			
			// get the pushed vertices
			return std::move(u.discoveredVertices);
		}));

		iters++;
		
		// calculate all "work" done by all vertices in active
		flow.workSinceLastGR += parlay::reduce(parlay::map(flow.active, [&] (vertex_id ui) {
			return flow.vertices[ui].work;
		}));
		
		// if total work since last relabel exceeds threshold from hi_pr, 
		// do a global relabel
		if(flow.workSinceLastGR * flow.freq > flow.alpha * flow.n + flow.m) {
			flow.workSinceLastGR = 0; 
			flow.global_relabel();
		}
	}

	flow.check_correctness();

	// note: this is only a maximum preflow, and not a max flow 
	// this is sufficient to derive a min cut, which can be used to find the 
	// max flow if needed
	return flow.vertices[flow.sink].excess;
}

// from the push relabel example
void FIFOSyncParPR::check_correctness() {
	auto total = parlay::reduce(parlay::tabulate(n, [&] (int vi) {
		Vertex& v = vertices[vi];
		long total_flow = parlay::reduce(parlay::map(v.edges, [] (Edge e) {
			return e.flow;}));
		if (vi != source && total_flow != -v.excess) {
			std::cout << "flow does not match excess at " << vi << std::endl; abort();}
		long capacity_failed = parlay::reduce(parlay::map(v.edges, [] (Edge e) {
			return (int) e.flow > e.capacity;}));
		if (capacity_failed > 0) {
			std::cout << "capacity oversubsribed from: " << vi << std::endl; abort();}
		long invalid_label = parlay::reduce(parlay::map(v.edges, [&] (Edge e) {
			return (e.flow < e.capacity && v.label > vertices[e.v].label + 1);}));
		if (invalid_label > 0) {
			std::cout << "invalid label at: " << vi << std::endl; abort();}
		if (v.label != 0 && v.label < n && v.excess > 0) {
			std::cout << "left over excess at " << vi
								<< "excess = " << v.excess << "label = " << v.label << std::endl;
			abort();
		}
		return v.excess;}));

	int expected = std::numeric_limits<int>::max();
	if (total != expected)
		std::cout << "excess lost: " << expected-total << std::endl;
}
