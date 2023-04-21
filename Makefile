all: avg-mpi1 # avg-sr

avg-sr:
	gcc average-serial.c -o  average-serial
	./average-serial input

avg-mpi1:
	mpicc average-mpi-ppv1.c -o  average-mpi-ppv1
	mpirun --oversubscribe -np 5 average-mpi-ppv1 inputa8
	

clear:
	rm  average-serial a.out