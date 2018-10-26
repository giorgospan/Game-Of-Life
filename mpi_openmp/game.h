#ifndef GAME_H
#define GAME_H

/*Creates a subarray datatype according to the parameters given*/
void create_datatype(MPI_Datatype* ,int ,int ,int ,int );

/*Calculates next state of (i,j) cell according to its
neighbourhood's sum */
int find_next_state(int ,int ,int );


/*Finds the sum the 8 neighbours*/
void find_neighbourhood_sum(int ,int ,int* );


/*Calculation for outer matrix after communication is completed*/
void calculate_outer_matrix(void);

/*Calculation for inner_matrix which does not require communication*/
/*This is used to overlap commmunication time*/
void calculate_inner_matrix(void);

/*Finds the rank of every neighbour of the calling process*/
void find_neighbours(MPI_Comm,int,int,int,int* ,int* ,int* ,int* ,int* ,int* ,int* ,int* );

/*Checks if there has been no change in any of the processes*/
int termination_check(MPI_Comm,int);

/*Prints each neighbour's rank*/
void print_neighbours(int,int ,int ,int ,int ,int ,int ,int ,int );

/*Prints local matrix of the calling process*/
void print_local_matrix();

/*Game of life */
void game(MPI_Comm ,int ,int,int,int );

#endif