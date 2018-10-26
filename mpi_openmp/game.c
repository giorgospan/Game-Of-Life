#include <stdio.h> /*for printf()*/
#include <stdlib.h>/*for rand(),malloc(),free()*/
#include <mpi.h> /*for MPI functions*/
#include <unistd.h> /*for sleep() [debugging purposes]*/

/*for OpenMP functions*/
#ifdef _OPENMP
#include <omp.h>
#endif


#include "game.h"
#include "functions.h"
#include "misc_header.h"

#define TAG 0

/*Visible to every function in this source file*/
char** next_gen;
int changed_cells;

void create_datatype(MPI_Datatype* derivedtype,int start1,int start2,int subsize1,int subsize2)
{
	const int array_of_bigsizes[2] = {local_N+2,local_M+2};
	const int array_of_subsizes[2] = {subsize1,subsize2};
	const int array_of_starts[2] = {start1,start2};

	MPI_Type_create_subarray(2,array_of_bigsizes,array_of_subsizes,array_of_starts,MPI_ORDER_C, MPI_CHAR,derivedtype);
	MPI_Type_commit(derivedtype);
}

int find_next_state(int i,int j,int sum)
{
	int local_changed_cells = 0;
	
	/*If cell is alive*/
	if(local_matrix[i][j]=='1')
	{
		if(sum == 0 || sum == 1 )
		{
			next_gen[i][j] = '0';
			local_changed_cells++;
		}
		else if(sum == 2 || sum == 3)
			next_gen[i][j] = '1';
			
		else if(sum>=4 && sum<=8)
		{
			next_gen[i][j] = '0';
			local_changed_cells++;
		}
			
	}
	/*If cell is not alive and has 3 active neighbours*/
	else if(sum == 3)
	{
		next_gen[i][j] = '1';
		local_changed_cells++;
	}
	/*If cell is not alive but it has less than 3 active neighbours*
	/*Then it remains alive*/
	else
		next_gen[i][j] = '0';
	return local_changed_cells;
}

void find_neighbourhood_sum(int current_i,int current_j,int* sum)
{
	int i,j;
	
	*sum=0;
	
	/*For all my 8 neighbours*/
	for(i=-1;i<=1;++i)
	{
		for(j=-1;j<=1;++j)
		{
			if(i || j)
				/*If neighbour is alive,add it to sum*/
				if(local_matrix[current_i+i][current_j+j]=='1')
					(*sum)++;
		}
	}
}

void calculate_inner_matrix(void)
{
	
	/*For all cells that require no communication at all*/
	int n;
	changed_cells = 0;
	#pragma omp for schedule(static) reduction(+:changed_cells)
	for(n=0;n<(local_N-2)*(local_M-2);++n)
	{
		int i,j,sum;
		i=n/(local_M-2)+2;
		j=n%(local_M-2)+2;
		find_neighbourhood_sum(i,j,&sum);
		changed_cells+=find_next_state(i,j,sum);
	}
}


void calculate_outer_matrix(void)
{
	int i,j;
	int sum;
	/*For all the border-cells*/
	for(i=1;i<=local_N;++i)
		for(j=1;j<=local_M;++j)
		{
			if(i==1 || i==local_N || j==1 || j==local_M)
			{
				find_neighbourhood_sum(i,j,&sum);
				find_next_state(i,j,sum);
			}
		}
}


void find_neighbours(MPI_Comm comm_2D,int my_rank,int NPROWS,int NPCOLS,int* left,int* right,int* top,int* bottom,int* topleft,int* topright,int* bottomleft,int* bottomright)
{
	
	int source,dest,disp=1;
	int my_coords[2];
	int corner_coords[2];
	int corner_rank;
	
	
	/*Finding top/bottom neighbours*/
	MPI_Cart_shift(comm_2D,0,disp,top,bottom);
	
	/*Finding left/right neighbours*/
	MPI_Cart_shift(comm_2D,1,disp,left,right);
	
	/*Finding top-right corner*/
	MPI_Cart_coords(comm_2D,my_rank,2,my_coords);
	corner_coords[0] = my_coords[0] -1; 
	corner_coords[1] = (my_coords[1] + 1) % NPCOLS ;
	if(corner_coords[0] < 0)
		corner_coords[0] = NPROWS -1;
	MPI_Cart_rank(comm_2D,corner_coords,topright);
	
	/*Finding top-left corner*/
	MPI_Cart_coords(comm_2D,my_rank,2,my_coords);
	corner_coords[0] = my_coords[0] - 1; 
	corner_coords[1] = my_coords[1] - 1 ; 
	if(corner_coords[0]<0)
		corner_coords[0] = NPROWS -1;
	if (corner_coords[1]<0)
		corner_coords[1] = NPCOLS -1;
	MPI_Cart_rank(comm_2D,corner_coords,topleft);
	
	/*Finding bottom-right corner*/
	MPI_Cart_coords(comm_2D,my_rank,2,my_coords);
	corner_coords[0] = (my_coords[0] + 1) % NPROWS ; 
	corner_coords[1] = (my_coords[1] + 1) % NPCOLS ; 
	MPI_Cart_rank(comm_2D,corner_coords,bottomright);
	
	/*Finding bottom-left corner*/
	MPI_Cart_coords(comm_2D,my_rank,2,my_coords);
	corner_coords[0] = (my_coords[0] + 1) % NPROWS ; 
	corner_coords[1] = my_coords[1] - 1 ;
	if (corner_coords[1]<0)
		corner_coords[1] = NPCOLS -1;
	MPI_Cart_rank(comm_2D,corner_coords,bottomleft);
	
}

int termination_check(MPI_Comm comm_2D,int my_rank)
{
	int sum;
	int changed;
	
	if(changed_cells)
		changed = 1;
	else changed = 0;
	
	MPI_Allreduce(&changed,&sum,1,MPI_INT,MPI_SUM,comm_2D);
	
	if(sum == 0)
		return 1;
	else 
		return 0;
}

void print_local_matrix(void)
{
	
	int i,j;
	for(i=1;i<=local_N;++i)
	{
		for(j=1;j<=local_M;++j)
			printf("%c",local_matrix[i][j]);
		printf("\n");
	}
}


void print_neighbours(int my_rank,int left,int right,int top,int bottom,int topleft,int topright,int bottomleft,int bottomright)
{
	
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if(my_rank==rank)
	{
		printf("My_rank:%d\n============\n",my_rank);
		printf("Left:%d\n",left);
		printf("Right:%d\n",right);
		printf("Top:%d\n",top);
		printf("Bottom:%d\n",bottom);
		printf("TopLeft:%d\n",topleft);
		printf("TopRight:%d\n",topright);
		printf("BottomLeft:%d\n",bottomleft);
		printf("BottomRight:%d\n=============\n",bottomright);
	}
}

void game(MPI_Comm comm_2D,int my_rank,int NPROWS,int NPCOLS,int MAX_GENS)
{

	int gen,i,j;
	next_gen = allocate_memory(local_N+2,local_M+2);
	char** temp;
	
	
	/*16 requests , 16 statuses */
	MPI_Request array_of_requests[16];
	MPI_Status array_of_statuses[16];
	
	
	/*Create 4 datatypes for sending*/
	MPI_Datatype firstcolumn_send,firstrow_send,lastcolumn_send,lastrow_send;
	create_datatype(&firstcolumn_send,1,1,local_N,1);
	create_datatype(&firstrow_send,1,1,1,local_M);
	create_datatype(&lastcolumn_send,1,local_M,local_N,1);
	create_datatype(&lastrow_send,local_N,1,1,local_M);
	
	/*Create 4 datatypes for receiving*/
	MPI_Datatype firstcolumn_recv,firstrow_recv,lastcolumn_recv,lastrow_recv;
	create_datatype(&firstcolumn_recv,1,0,local_N,1);
	create_datatype(&firstrow_recv,0,1,1,local_M);
	create_datatype(&lastcolumn_recv,1,local_M+1,local_N,1);
	create_datatype(&lastrow_recv,local_N+1,1,1,local_M);
	
	/*Find ranks of my 8 neighbours*/
	int left,right,bottom,top,topleft,topright,bottomleft,bottomright;
	find_neighbours(comm_2D,my_rank,NPROWS,NPCOLS,&left,&right,&top,&bottom,&topleft,&topright,&bottomleft,&bottomright);
	
	MPI_Send_init(&(local_matrix[0][0]),1,				firstcolumn_send,left,			TAG,comm_2D,&array_of_requests[0]);
	MPI_Send_init(&(local_matrix[0][0]),1,				firstrow_send,	top,			TAG,comm_2D,&array_of_requests[1]);
	MPI_Send_init(&(local_matrix[0][0]),1,				lastcolumn_send,right,			TAG,comm_2D,&array_of_requests[2]);
	MPI_Send_init(&(local_matrix[0][0]),1,				lastrow_send,	bottom,			TAG,comm_2D,&array_of_requests[3]);
	MPI_Send_init(&(local_matrix[1][1]),1,				MPI_CHAR,		topleft,		TAG,comm_2D,&array_of_requests[4]);
	MPI_Send_init(&(local_matrix[1][local_M]),1,		MPI_CHAR,		topright,		TAG,comm_2D,&array_of_requests[5]);
	MPI_Send_init(&(local_matrix[local_N][local_M]),1,	MPI_CHAR,		bottomright,	TAG,comm_2D,&array_of_requests[6]);
	MPI_Send_init(&(local_matrix[local_N][1]),1,		MPI_CHAR,		bottomleft,		TAG,comm_2D,&array_of_requests[7]);
	
	MPI_Recv_init(&(local_matrix[0][0]),1,				firstcolumn_recv,left,			TAG,comm_2D,&array_of_requests[8]);
	MPI_Recv_init(&(local_matrix[0][0]),1,				firstrow_recv,	top,			TAG,comm_2D,&array_of_requests[9]);
	MPI_Recv_init(&(local_matrix[0][0]),1,				lastcolumn_recv,right,			TAG,comm_2D,&array_of_requests[10]);
	MPI_Recv_init(&(local_matrix[0][0]),1,				lastrow_recv,	bottom,			TAG,comm_2D,&array_of_requests[11]);
	MPI_Recv_init(&(local_matrix[0][0]),1,				MPI_CHAR,		topleft,		TAG,comm_2D,&array_of_requests[12]);
	MPI_Recv_init(&(local_matrix[0][local_M+1]),1,		MPI_CHAR,		topright,		TAG,comm_2D,&array_of_requests[13]);
	MPI_Recv_init(&(local_matrix[local_N+1][local_M+1]),1,MPI_CHAR,		bottomright,	TAG,comm_2D,&array_of_requests[14]);
	MPI_Recv_init(&(local_matrix[local_N+1][0]),1,		MPI_CHAR,		bottomleft,		TAG,comm_2D,&array_of_requests[15]);
	
	
	for(gen=0;gen<MAX_GENS;gen++)
	{
		/*Uncomment following lines if you want to see the generations of process with
		rank "my_rank" evolving*/
		
		
		// if(my_rank==0)
		// {
			// printf("Generation:%d\n",gen+1);
			// for(i=0;i<local_M;++i)
				// putchar('~');
			// putchar('\n');
			// print_local_matrix();
		// }
		
		/*Start all requests [8 sends + 8 receives]*/
		MPI_Startall(16,array_of_requests);
		
		/*Overlap communication [calculating inner matrix]*/
		calculate_inner_matrix();
		
		/*Make sure all requests are completed*/
		MPI_Waitall(16,array_of_requests,array_of_statuses);
		
		/*We are ready to calculate the outer matrix*/
		calculate_outer_matrix();
		
		/*Check if it has remained the same using a flag*/
		if(termination_check(comm_2D,my_rank))
		{
			if(!my_rank)
				printf("No change on %d generation\n",gen);
			break;
		}
	
		/*next_gen will become our local matrix[=our current gen]*/
		temp = local_matrix;
		local_matrix = next_gen;
		next_gen = temp;
	}
		
	
	// if(changed && !my_rank)
		// printf("Termination after MAXGENS[%d] iterations\n",MAX_GENS);

	
	/*Free resources*/
	MPI_Type_free(&firstcolumn_send);
	MPI_Type_free(&firstrow_send);
	MPI_Type_free(&lastcolumn_send);
	MPI_Type_free(&lastrow_send);
	
	MPI_Type_free(&firstcolumn_recv);
	MPI_Type_free(&firstrow_recv);
	MPI_Type_free(&lastcolumn_recv);
	MPI_Type_free(&lastrow_recv);
	
	free(next_gen[0]);
	free(next_gen);
}







