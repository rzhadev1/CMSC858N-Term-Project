'''
Convert a DIMACS graph file to compressed sparse row format
'''
import os
import numpy as np

GRAPH_TYPE = "WeightedAdjacencyGraph" # type of the graph (for max flow, this is always weighted adjacency)
SRC_PATH = os.path.abspath('..')  # should be 1 levels above the parser
DIMACS_FOLDER = 'data/dimacs'
DIMACS_FOLDER_PATH = os.path.join(SRC_PATH, DIMACS_FOLDER)

subfolders = [f.path for f in os.scandir(DIMACS_FOLDER_PATH) if f.is_dir()]
for subfolder in subfolders: 
	
	name = os.path.basename(subfolder)  # name of the vision instance
	max_file = name + '.max'
	max_file_path = os.path.join(subfolder, max_file)
	
	n = 0 # number of vertices
	m = 0 # number of edges
	source = 0 # source vertex label
	sink = 0 # sink vertex label

	# parlay stores a weighted graph as: 
	# header n m 
	with open(max_file_path) as file:

		for line in file: # don't read all lines into memory at once
			
			if line == 'c': 
				continue

			elif line == 'p': # problem statement
				tokens = line.split(' ')
				n = int(tokens[2])
				m = int(tokens[3])

			elif line == 'n': 
				pass

