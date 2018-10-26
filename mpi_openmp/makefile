OBJS	= main.o game.o functions.o
SOURCE	= main.c game.c functions.c
HEADERS	= game.h functions.h

OUT		= gameoflife

CC 		= mpicc
FLAGS 	= -g -c -fopenmp


all: clean game aux


#### Create .exe for auxilary program ####
aux: grid_creator.o
	gcc -g grid_creator.o -o grid_creator

	
#### Create .exe for main program ###
game: $(OBJS)
	$(CC) -g -fopenmp -o $(OUT) $(OBJS) 

	
#####  Our main program #####	
main.o: main.c
	$(CC) $(FLAGS) main.c
	
game.o: game.c
	$(CC) $(FLAGS) game.c

functions.o: functions.c
	$(CC) $(FLAGS) functions.c
	
	
###### Auxilary program #########	
grid_creator.o: grid_creator.c
	gcc -g -c grid_creator.c
	
	
	
#clean house#
clean:
	rm -f grid_creator.o grid_creator $(OBJS) $(OUT)
