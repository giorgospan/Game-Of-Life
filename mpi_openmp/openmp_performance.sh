#!/bin/bash


#Create executable
make

#max generations
max_gen=10;


for processes in 1 
	do
	echo Number of Processes : $processes
	echo ================================
	for N in 2000 4000 8000
		do
			echo Grid Dimensions : $N x $N
			echo --------------------------------
			for threads in 1 2 4 8
				do
					echo Threads: $threads
					mpiexec -n $processes -f ./machines ./gameoflife -n $N -m $N -max $max_gen -t $threads
				done
		done
	done
echo

