#ifndef FUNCTIONS_H
#define FUNCTIONS_H


/*Allocates contiguous memory using malloc*/
/*Important in order work with MPI_Datatypes*/
char** allocate_memory(int ,int );


/*Calculates number of rows & columns for each process*/
void calculate_rows_columns(int*,int* ,int ,int ,int ,int ,int );


/*Calculates disp for each process*/
void calculate_disp(int* ,int* ,int* ,int,int ,int );


/*Calculates extent for each process*/
void calculate_extent(int* ,int* ,int ,int );

/*Reads the input file [if provided as an argument]*/
void read_file(char*,int ,int,int );

#endif