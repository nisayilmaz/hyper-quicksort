all: hyperqsort qsort

hyperqsort:
	mpicc hyperqsort.c -o hyperqsort -lm

qsort:
	gcc qsort.c -o qsort

clean:
	rm -rf hyperqsort qsort 