all: bmpi bsr  avg-mpi2  avg-mpi1  avg-sr

avg-sr:
	@gcc average-serial.c -o  average-serial
# ./average-serial input

avg-mpi1: average-mpi-ppv1.c
	@mpicc average-mpi-ppv1.c -o  average-mpi-ppv1
# mpirun --oversubscribe -np 4 average-mpi-ppv1 inputa8


avg-mpi2: average-mpi-ppv2.c
	@mpicc average-mpi-ppv2.c -o  average-mpi-ppv2
# mpirun --oversubscribe -np 4 average-mpi-ppv2 inputa8
	
bsr:bucket_average-serial.c
	@gcc bucket_average-serial.c -o  bucket_average-serial
# ./bucket_average-serial inputb outputbsr.txt

bmpi:bucket_average-mpi.c
	@mpicc bucket_average-mpi.c -o  bucket_average-mpi
# mpirun --oversubscribe -np 3 bucket_average-mpi inputb outputbmpi.txt

clear:
	@rm -f bucket_average-mpi bucket_average-serial average-mpi-ppv1 average-mpi-ppv2 average-serial a.out

c: clear

z: zip

zip:
	zip -r muhammed_can_kucukaslan.zip muhammed_can_kucukaslan_cs426_pj1.pdf Makefile average-serial.c average-mpi-ppv1.c average-mpi-ppv2.c bucket_average-serial.c bucket_average-mpi.c

100k:
	time ./average-serial a_100_000
	time mpirun --oversubscribe -np 16 average-mpi-ppv1 a_100_000
	time mpirun --oversubscribe -np 16 average-mpi-ppv2 a_100_000
	time mpirun --oversubscribe -np 32 average-mpi-ppv1 a_100_000
	time mpirun --oversubscribe -np 32 average-mpi-ppv2 a_100_000

10m:
	time ./average-serial a_10_000_000
	time mpirun --oversubscribe -np 16 average-mpi-ppv1 a_10_000_000
	time mpirun --oversubscribe -np 16 average-mpi-ppv2 a_10_000_000
	time mpirun --oversubscribe -np 32 average-mpi-ppv1 a_10_000_000
	time mpirun --oversubscribe -np 32 average-mpi-ppv2 a_10_000_000


