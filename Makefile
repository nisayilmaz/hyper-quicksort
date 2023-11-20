all: parallel sequential

parallel:
	mpicc parallel.c -o parallel -lm

sequential:
	gcc sequential.c -o sequential

clean:
	rm -rf parallel sequential