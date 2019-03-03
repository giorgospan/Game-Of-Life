# source files
MPI_SOURCE        = $(wildcard ./mpi/*.c)
MPI_OPENMP_SOURCE = $(wildcard ./mpi_openmp/*.c)

# object files
MPI_OBJ           = $(MPI_SOURCE:%.c=%.o)
MPI_OPENMP_OBJ    = $(MPI_OPENMP_SOURCE:%.c=%.o)

# outputs
MPI_OUT           = ./mpi/gameoflife
MPI_OPENMP_OUT    = ./mpi_openmp/gameoflife

MPICC             = mpicc
CC                = gcc
FLAGS             = -g -O3

# Auxialiary program
grid:aux/grid
aux/grid:aux/grid_creator.o
	$(CC) -o $@ $^

aux/grid_creator.o:aux/grid_creator.c
	$(CC) $(FLAGS) -c $^ -o $@

# MPI
mpi:clean_mpi $(MPI_OUT)
$(MPI_OUT):$(MPI_OBJ)
	$(MPICC) -o $@ $^

$(MPI_OBJ):./mpi/%.o : ./mpi/%.c
	$(MPICC) $(FLAGS) -c $< -o $@

# MPI + OpenMP
openmp:clean_openmp $(MPI_OPENMP_OUT)
$(MPI_OPENMP_OUT):$(MPI_OPENMP_OBJ)
	$(MPICC) -fopenmp -o $@ $^

$(MPI_OPENMP_OBJ):./mpi_openmp/%.o : ./mpi_openmp/%.c
	$(MPICC) $(FLAGS) -c $< -o $@ -fopenmp

# Cleaning
.PHONY:clean_all
clean_all:clean_mpi clean_openmp
	rm -f $(MPI_OBJ)

.PHONY:clean_mpi
clean_mpi:
	rm -f $(MPI_OBJ) $(MPI_OUT)

.PHONY:clean_openmp
clean_openmp:
		rm -f $(MPI_OPENMP_OBJ) $(MPI_OPENMP_OUT)