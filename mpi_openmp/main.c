#include <stdio.h> /*for printf()*/
#include <stdlib.h>/*for rand(),malloc(),free()*/
#include <string.h> /*for strcmp()*/
#include <unistd.h> /*for atoi()*/
#include <mpi.h> /*for MPI functions*/

/*for OpenMP functions*/
#ifdef _OPENMP
#include <omp.h>
#endif

#include "functions.h"
#include "game.h"
#include "misc_header.h"


char** local_matrix;
int local_N;
int local_M;
int thread_count;

int main(int argc, char **argv) 
{
/************************************************************************************************************/
    int size, rank, i, j, proc,COLS,ROWS,MAX_GENS;
	int flag1=0,flag2=0,flag3=0,flag4=0,flag5=0;
	double local_start,local_finish,local_elapsed,elapsed;
	char* filename;
	
    MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

	for(i=0;i<argc;++i)
	{
		if(!strcmp("-n",argv[i]))
		{
			ROWS = atoi(argv[i+1]);
			flag1=1;
		}
		else if(!strcmp("-m",argv[i]))
		{
			COLS = atoi(argv[i+1]);
			flag2=1;
		}	
		else if(!strcmp("-max",argv[i]))
		{
			
			MAX_GENS = atoi(argv[i+1]);
			flag3=1;
		}

		else if(!strcmp("-f",argv[i]))
		{
			filename = malloc((strlen(argv[i+1]+1))* sizeof(char));
			strcpy(filename,argv[i+1]);
			flag4=1;
		}
		else if(!strcmp("-t",argv[i]))
		{
			thread_count = atoi(argv[i+1]);
			flag5=1;
		}
	}
	
	/*Sanity Check*/
	if(!flag1 || !flag2 || !flag3 || !flag5)
	{
		if(rank == 0)
			printf("Usage:mpiexec [-n <NoPROCESSES>] [-f <machine_file>] ./gol -n <ROWS> -m <COLUMNS> -max <MAX_GENS> -t <threads>[-f <inputfile>] \nExiting...\n\n");
		MPI_Finalize();
		exit(1);
	}
	
/************************************************************************************************************/
    /* Setup virtual 2D topology*/
    int dims[2] = {0,0};
    MPI_Dims_create(size, 2, dims);
    int periods[2] = {1,1}; /*Periodicity in both dimensions*/
    int my_coords[2];
    MPI_Comm comm_2D;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &comm_2D);
    MPI_Cart_coords(comm_2D, rank, 2, my_coords);

    const int NPROWS = dims[0]; /* Number of 'block' rows */
    const int NPCOLS = dims[1]; /* Number of 'block' cols */
    int *num_rows; /* Number of rows for the i-th process [local_N]*/
    int *num_cols; /* Number of columns for the i-th process [local_M]*/
	int *extent;  /* Extent for the i-th process [local_extent]*/
	int *disps; /*Displacement for the i-th process [local_disp]*/
	
	int local_disp;
	int local_extent;
	
/************************************************************************************************************/	
	// char name[MPI_MAX_PROCESSOR_NAME];
	// int namelen = MPI_MAX_PROCESSOR_NAME;
	// MPI_Get_processor_name(name,&namelen);
	// printf("Process:%d | Machine:%s\n",my_rank+1,name);
	
	
	/*Start timing*/
	MPI_Barrier(comm_2D);
	local_start = MPI_Wtime();
	

	/* Calculate rows,cols,displacement,extent for each process */
    if (rank == 0) 
	{
		
		num_rows = (int *) malloc(size * sizeof(int));
		num_cols = (int *) malloc(size * sizeof(int));
		
		
		calculate_rows_columns(num_rows,num_cols,size,ROWS,COLS,NPROWS,NPCOLS);
		
		if(flag4)
		{
			disps = (int *) malloc(size * sizeof(int));
			extent = (int *) malloc(size * sizeof(int));
			
			calculate_disp(disps,num_rows,num_cols,COLS,NPROWS,NPCOLS);
			calculate_extent(extent,num_cols,NPROWS,NPCOLS);
		}
	}

/************************************************************************************************************/	
	
	// Scatter dimensions,displacement,extent of each process 
	MPI_Scatter(num_rows,1,MPI_INT,&local_N,1,MPI_INT,0,comm_2D);
	MPI_Scatter(num_cols,1,MPI_INT,&local_M,1,MPI_INT,0,comm_2D);
	
	if(rank == 0)
	{free(num_rows);free(num_cols);}
	
	if(flag4)
	{
		MPI_Scatter(disps,1,MPI_INT,&local_disp,1,MPI_INT,0,comm_2D);
		MPI_Scatter(extent,1,MPI_INT,&local_extent,1,MPI_INT,0,comm_2D);
		
		if(rank == 0)
		{free(disps);free(extent);}
	}
	
/************************************************************************************************************/
	
	
	local_matrix = allocate_memory(local_N+2,local_M+2);
		
	/*If a file has been provided*/
	if(flag4)
		read_file(filename,local_disp,local_extent,rank);
	/*If no file has been provided*/
	/*Each process will fill its own submatrix randomly*/
	else
	{
		int n;
		// # pragma omp parallel for num_threads(thread_count) schedule(dynamic)
			for(n=0;n<local_N*local_M;++n)
			{
				int i,j;
				i=n/local_M+1;
				j=n%local_M+1;
				if(rand() % 2)
					local_matrix[i][j] = '1';
				else
					local_matrix[i][j] = '0';
			}
	}

	/*Each process will start the game*/
	game(comm_2D,rank,NPROWS,NPCOLS,MAX_GENS);
	
	/*Getting finish time*/
	local_finish = MPI_Wtime();
	local_elapsed = local_finish - local_start;
	MPI_Reduce(&local_elapsed,&elapsed,1,MPI_DOUBLE,MPI_MAX,0,comm_2D);
	
	
	if(!rank)
		printf("Elapsed time:%.3f seconds\n",elapsed);
	
	free(local_matrix[0]);
	free(local_matrix);

	MPI_Finalize();
	
}	
