#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>


int partition(int * arr, int p, int r) {
    int pivot = arr[p];
    int i = p ;
    int j = r ;
    while (1){
        while (arr[i] < pivot) {
            i++;
        }
        while (arr[j] > pivot) {
            j--;
        }

        if (i < j ) {
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
        else return j;

    }
}

void quicksort(int * arr, int p, int r){
    if(p < r) {
    
        int q = partition(arr, p, r);
        quicksort(arr, p, q);
        quicksort(arr, q + 1, r);
    }
   
}

int main(int argc, char* argv[]) {

    int myid, numprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    int * arr;
    int size;

    if(myid == 0) {
        FILE *fileptr = fopen("input.txt", "r");
        arr = NULL;
        int num;
        size = 0;

        while(fscanf(fileptr, "%d", &num ) == 1) {
            arr = realloc(arr, (size + 1) * sizeof(int));
            arr[size++] = num;
        }

        fclose(fileptr);
    }

    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int workload = size / numprocs;
    int * myarr = malloc(workload * sizeof(int));
    MPI_Scatter(arr, workload, MPI_INT, myarr, workload, MPI_INT, 0, MPI_COMM_WORLD);

    quicksort(myarr, 0, workload - 1);
 

    int median;
    if( workload % 2 != 0) {
        median = myarr[(workload - 1) / 2];
    }
    else {
        median = (myarr[workload/2] + myarr[(workload / 2) - 1]) / 2;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    int * medians = malloc(numprocs * sizeof(int));
    MPI_Allgather(&median, 1, MPI_INT, medians, 1, MPI_INT, MPI_COMM_WORLD);
    int medianOfMedians;
    
    if( numprocs % 2 != 0) {
        medianOfMedians = medians[(numprocs - 1) / 2];
    }
    else {
        medianOfMedians = (medians[numprocs/2] + medians[(numprocs / 2) - 1]) / 2;
    }
    printf("mm:%d\n", medianOfMedians);

    int dimension = 3;
    int mask = pow(2, dimension - 1);
    int myPartner = myid ^mask;
    int maskedId = myid & mask;
    int color, sublistCount = 0, index;
    int key = myid;
    MPI_Status status;
    if(maskedId == 0) {
        color = 1;
        for(int i = workload - 1; i >= 0; i--){
            if(myarr[i] >= medianOfMedians){
                sublistCount++;
                index = i;
            }
            else {
                break;
            }
        }

        MPI_Send(&sublistCount, 1, MPI_INT, myPartner, 0, MPI_COMM_WORLD);
        MPI_Send(&myarr[index], sublistCount, MPI_INT, myPartner, 1, MPI_COMM_WORLD);
        printf("my id:%d, sent to my partner %d\n", myid, myPartner);
        for (int i = index; i < index + sublistCount; i++)
        {
            printf("%d ", myarr[i]);
        }
        printf("\n");


    }
    else {
        color = 0;
        MPI_Recv(&sublistCount, 1, MPI_INT, myPartner, 0, MPI_COMM_WORLD, &status);
        int * recvd = malloc(sublistCount * sizeof(int));
        MPI_Recv(&recvd[0], sublistCount, MPI_INT, myPartner, 1, MPI_COMM_WORLD, &status);
        printf("received from my partner %d\n", myPartner);
        for (int i = 0; i < sublistCount; i++)
        {
            printf("%d ", recvd[i]);
        }
        printf("\n");
        free(recvd);


    }

    /* MPI_Comm new_comm;
    MPI_Comm_split(MPI_COMM_WORLD, color, key, &new_comm);
    int newcommsize, newid;

    MPI_Comm_size(new_comm, &newcommsize);
    MPI_Comm_rank(new_comm, &newid);
    printf("myid: %d, commsize: %d\n", newid, newcommsize); */
    
    if(myid == 0) {
        free(arr);
    }
    free(myarr);
    free(medians);
    MPI_Finalize();


    return 0;
}