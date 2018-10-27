# Game of Life

A parallel implementation of [Conway's Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life "Wikipedia") .

This project was part of my "Parallel Systems" course during fall semester 2016.

MPI and OpenMP are being used to achieve parallelism.

"machines" file contains a list with entries of the following type: *<machine_name>:<number_of_cores_to_be_used>*

Run MPI only version:
  * mpiexec \[-n \<NoPROCESSES>] \[-f \<machine_file>] ./gameoflife -n \<ROWS> -m \<COLUMNS> -max \<MAX_GENS> \[-f \<inputfile>]


Run MPI+OpenMP version:
  * mpiexec \[-n \<NoPROCESSES>] \[-f \<machine_file>] ./gol -n \<ROWS> -m \<COLUMNS> -max \<MAX_GENS> -t \<threads> \[-f \<inputfile>]
