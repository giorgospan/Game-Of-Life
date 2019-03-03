#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h> // to have access to flags def
#define PERMS 0644 // set access permissions


char filename[] = "inputfile";

int main(int argc,char* argv[])
{

	int choice;
	int N,M,i,j;
	int fd;
	int n;
	char c;

	printf("Enter number of rows:");
	if(scanf("%d",&N)<1)
	{
		perror("scanf");
		exit(EXIT_FAILURE);
	}
	putchar('\n');

	printf("Enter number of columns:");
	if(scanf("%d",&M)<1)
	{
		perror("scanf");
		exit(EXIT_FAILURE);
	}

	putchar('\n');

	printf(
	"1.Enter your own grid values\n"
	"2.Let rand() decide for the grid values\n"
	"Press \"1\" or \"2\":\n");
	if(scanf("%d",&choice)<1)
	{
		perror("scanf");
		exit(EXIT_FAILURE);
	}

	/*Create an inputfile*/
	if (( fd = open ( filename , O_CREAT | O_RDWR | O_TRUNC , PERMS ))== -1)
	{
		perror ("Creating file");
		exit (1) ;
	}

	/*Fill in the inputfile*/
	for(i=0;i<N;++i)
	{
		for(j=0;j<M;++j)
		{

			if(choice==1)
			{
				printf("Enter grid[%d][%d]:",i,j);
				if( (n=scanf(" %c",&c)) < 1 )
				{
					perror("scanf()");
					exit(EXIT_FAILURE);
				}
				putchar('\n');
			}
			else if(rand()%2)
				c='1';
			else
				c='0';

			if(write(fd,&c,1) != 1)
			{
				perror("Writing to file");
				exit(EXIT_FAILURE);
			}
		}
		if(write(fd,"\n",1) != 1)
		{
			perror("Writing to file");
			exit(EXIT_FAILURE);
		}
	}
	close(fd);

}