# CMSC858N-Term-Project

## Requirements
Boost 1.82.0, cmake 3.14, c++17. 

## Usage 
```
mkdir build
cd build
cmake ..
make
```
To run the parallel max flow solver, use 
```
syncPar <filename>
```
To run the sequential baseline solver, use 
```
bk <filename>
```

## Inputs

The solver is only able to parse graphs in DIMACS max flow format. See [this link](https://lpsolve.sourceforge.net/5.5/DIMACS_maxf.htm) for an explanation. 
The ```parser.h``` header file parses DIMACS graphs into parlaylib sequences, which is used by the parallel algorithm. Note that the inputs to the solver function have to be symmetric graphs, whereas DIMACS does not require symmetry. The parser implicitly handles this. 





