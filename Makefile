all: bmpi # bsr # avg-mpi2 # avg-mpi1 # avg-sr

avg-sr:
	@gcc average-serial.c -o  average-serial
	./average-serial input

avg-mpi1: average-mpi-ppv1.c
	@mpicc average-mpi-ppv1.c -o  average-mpi-ppv1
	mpirun --oversubscribe -np 4 average-mpi-ppv1 inputa8


avg-mpi2: average-mpi-ppv2.c
	@mpicc average-mpi-ppv2.c -o  average-mpi-ppv2
	mpirun --oversubscribe -np 4 average-mpi-ppv2 inputa8
	
bsr:bucket_average-serial.c
	@gcc bucket_average-serial.c -o  bucket_average-serial
	./bucket_average-serial inputb outputbsr.txt

bmpi:bucket_average-mpi.c
	mpicc bucket_average-mpi.c -o  bucket_average-mpi
	mpirun --oversubscribe -np 4 bucket_average-mpi inputb outputbmpi.txt

clear:
	@rm -f bucket_average-serial average-mpi-ppv1 average-mpi-ppv2 average-serial a.out

c: clear
