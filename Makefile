all: avg-mpi2 # avg-mpi1 # avg-sr

avg-sr:
	gcc average-serial.c -o  average-serial
	./average-serial input

avg-mpi1: average-mpi-ppv1.c
	mpicc average-mpi-ppv1.c -o  average-mpi-ppv1
	mpirun --oversubscribe -np 4 average-mpi-ppv1 inputa8


avg-mpi2: average-mpi-ppv2.c
	mpicc average-mpi-ppv2.c -o  average-mpi-ppv2
	mpirun --oversubscribe -np 4 average-mpi-ppv2 inputa8
	

clear:
	rm  average-mpi-ppv1 average-mpi-ppv2 average-serial a.out