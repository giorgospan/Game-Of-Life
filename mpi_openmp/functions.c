#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>


#include "functions.h"
#include "misc_header.h"


char** allocate_memory(int rows,int columns)
{
	int i;
	char *data = malloc(rows*columns*sizeof(char));
    char** arr = malloc(rows*sizeof(char *));
    for (i=0; i<rows; i++)
        arr[i] = &(data[i*columns]);
	
	return arr;
}

void calculate_rows_columns(int* num_rows,int* num_cols,int size,int ROWS,int COLS,int NPROWS,int NPCOLS)
{
	int i,j;
	for (i=0; i<size; i++) {
		num_rows[i] = ROWS/NPROWS;
		num_cols[i] = COLS/NPCOLS;
	}
	for (i=0; i<(ROWS%NPROWS); i++) {
		for (j=0; j<NPCOLS; j++) {
			num_rows[i*NPCOLS+j]++;
		}
	}
	for (i=0; i<(COLS%NPCOLS); i++) {
		for (j=0; j<NPROWS; j++) {
			num_cols[i+NPROWS*j]++;
		}
	}
}


void calculate_disp(int* disps,int* num_rows,int* num_cols,int COLS,int NPROWS,int NPCOLS)
{
	
	int i,j;
	
	for (i=0; i<NPROWS; i++)
		for (j=0; j<NPCOLS; j++) 
			if (j == 0)
			{
				int row;
				disps[i*NPCOLS+j] = 0;
				/*For all rows above me*/
				for(row=1;row<=i;row++)
				{
					disps[i*NPCOLS+j] +=  (COLS+1)*num_rows[i*NPCOLS+j - row*NPCOLS];
				}
			}
			else 
				/*Just add num_cols of the left process to its displacement*/
				disps[i*NPCOLS+j] = disps[i*NPCOLS+j - 1] + num_cols[i*NPCOLS+j - 1];
}

void calculate_extent(int* extent,int* num_cols,int NPROWS,int NPCOLS)
{
	int i,j;
	
	for (i=0; i<NPROWS; i++)
		for (j=0; j<NPCOLS; j++)
		{
			int current = i*NPCOLS+j;
			int block;
			
			/*Add newline to the extent*/
			/*Add num_cols of this process*/
			extent[current] = 1 + num_cols[current];
			
			/*For all blocks on the left of me*/
			for(block =i*NPCOLS ;block<current;block++)
				extent[current] += num_cols[block];
			/*For all blocks on the right */
			for(block = current+1;block<i*NPCOLS+NPCOLS;block++)
				extent[current] += num_cols[block];
		}
}



void read_file(char* filename,int local_disp,int local_extent,int rank)
{
	
	MPI_File fh;
	int i,j,error;
	
	/*Open the file*/
	MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
	error = MPI_File_open(MPI_COMM_WORLD,filename,MPI_MODE_RDONLY,MPI_INFO_NULL,&fh);
	
	
	if (error!= MPI_SUCCESS) 
	{
		/*Only Process 0 will print the error message */
		if(rank == 0)
		{
			char error_string[BUFSIZ];
			int length_of_error_string;

			MPI_Error_string(error, error_string, &length_of_error_string);
			fprintf(stderr,"%s\nExiting...\n\n",error_string);
		}

		/*Every process is about to exit*/
		MPI_Finalize();
		exit(1);
	}
	
	int lb=0;
	int count;
	int buffsize = local_N*local_M;
	char buff[buffsize];
	MPI_Datatype etype, filetype, contig;
	etype = MPI_CHAR;
	
	
	/*Create contiguous datatype of local_M chars*/
	MPI_Type_contiguous(local_M, MPI_CHAR, &contig);
	
	/*Extend this datatype*/
	MPI_Type_create_resized(contig, lb, (MPI_Aint)(local_extent) * sizeof(char), &filetype);
	
	/*Commit this datatype*/
	MPI_Type_commit(&filetype);
	
	/*Each process will now have its own file view*/
	MPI_File_set_view(fh, (MPI_Offset)(local_disp) * sizeof(char), etype, filetype,"native",MPI_INFO_NULL);
	
	/*At first we read all local_N*local_M characters in a temp buffer*/
	MPI_Status status;
	
	MPI_File_read_all(fh,buff,buffsize,MPI_CHAR,&status);
	MPI_Get_count( &status, MPI_CHAR, &count );
	
	if (count != buffsize) 
	{
		fprintf( stderr, "Read:%d instead of %d\n", count,buffsize );
		fflush(stderr);
		MPI_Finalize();
		exit(1);
	}
	else
	{
	/*We initialize our local matrix with the characters in the buffer*/
		int k=0;
		for(i=1;i<=local_N;++i)
			for(j=1;j<=local_M;++j)
			{
				local_matrix[i][j] = buff [k];
				k++;
			}
	}
	
	MPI_File_close(&fh);
}