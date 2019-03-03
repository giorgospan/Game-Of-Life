# Game of Life

## About
A parallel implementation of [Conway's Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life "Wikipedia") . MPI is used in order to leverage the processing power of many machines(nodes) working in parallel. In addition, we incorporate OpenMP directives for parallelization of *for loops*, thus creating a hybrid *MPI+OpenMp* implementation of the game.

## How to run

* MPI only :

  `make mpi`

  `cd mpi`

  `mpiexec [-n <NoPROCESSES>] [-f <machine_file>] ./gameoflife -n <ROWS> -m <COLUMNS> -max <MAX_GENS> [-f <inputfile>]`

* MPI+OpenMP :

  `make mpi_openmp`

  `cd mpi_openmp`

  `mpiexec [-n <NoPROCESSES>] [-f <machine_file>] ./gameoflife -n <ROWS> -m <COLUMNS> -max <MAX_GENS> -t threads [-f <inputfile>]`

### Notes

* You may create your own *initial generation* using the `make grid` command.

* In case no `inputfile` is given, the grid will be filled randomly.

* [machines](./machines) file contains a list with machine entries of the following format `<machine_name>:<number_of_cores_to_be_used>`


## Speedup

