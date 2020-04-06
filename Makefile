all:
	mpicc pract2.c -o pract2 -lX11

run:
	mpirun -np 1 ./pract2
