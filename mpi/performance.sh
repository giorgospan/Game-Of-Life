#!/bin/bash


#Create executable
make

max_gen=500;


# for processes in 1 2 4 6 8 10 12 
	# do
	# echo Number of Processes : $processes
	# echo --------------------------------
	# for N in 100 200 400 600 800 1200 2000 3000 4000
		# do
			# echo Grid Dimensions : $N x $N
			# if [ "$processes" -eq 1 ]; then
				# mpiexec -n $processes ./gameoflife -n $N -m $N -max $max_gen
			# else
				# mpiexec -n $processes -f ./machines ./gameoflife -n $N -m $N -max $max_gen
			# fi
		# done
	# done
# echo

processes=12;

echo Number of Processes : $processes
echo --------------------------------

for N in 100 200 400 600 800 1200 2000 
	do
		echo Grid Dimensions : $N x $N
			mpiexec -n $processes -f ./machines ./gameoflife -n $N -m $N -max $max_gen
	done