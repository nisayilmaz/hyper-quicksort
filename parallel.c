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
            j--;

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
    else return;
   
}

int main(int argc, char* argv[]) {

    int myid, numprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Status status;
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

    int dimension = 2;
    int mask = pow(2, dimension - 1);
    int myPartner = myid ^ mask;
    int maskedId = myid & mask;
    int color, sublistCount = 0, index, recvdSublistCount = 0,newSize;
    int key = myid;
    int * recvd = NULL;
    int * temp = NULL;
    if(maskedId == 0) {
        color = 0;
        index = workload;
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
        MPI_Recv(&recvdSublistCount, 1, MPI_INT, myPartner, 2, MPI_COMM_WORLD, &status);
        int * recvd = malloc(recvdSublistCount * sizeof(int));
        MPI_Recv(&recvd[0], recvdSublistCount, MPI_INT, myPartner, 3, MPI_COMM_WORLD, &status);

        newSize = index + recvdSublistCount;
        temp = malloc(newSize * sizeof(int));
        int newArrIndex = 0;
        for(int i = 0; i < index; i++) {
            temp[newArrIndex] = myarr[i];
            newArrIndex++;
        }
        for(int i = 0; i < recvdSublistCount; i++) {
            temp[newArrIndex] = recvd[i];
            newArrIndex++;
        }
        for(int i= 0; i < newSize; i++) {
            printf("id: %d, %dth element: %d \n", myid, i, temp[i]);
        } 
        if(newSize == 0) {
            printf("id: %d empty \n", myid);

        }   
        free(recvd);

    }
    else {
        color = 1;
        index = -1;
        MPI_Recv(&recvdSublistCount, 1, MPI_INT, myPartner, 0, MPI_COMM_WORLD, &status);
        recvd = malloc(recvdSublistCount * sizeof(int));
        MPI_Recv(&recvd[0], recvdSublistCount, MPI_INT, myPartner, 1, MPI_COMM_WORLD, &status);

        for(int i = 0; i < workload; i++){
            if(myarr[i] < medianOfMedians){
                sublistCount++; 
                index = i;
            }
            else {
                break;
            }
        }       
        MPI_Send(&sublistCount, 1, MPI_INT, myPartner, 2, MPI_COMM_WORLD);
        MPI_Send(&myarr[0], sublistCount, MPI_INT, myPartner, 3, MPI_COMM_WORLD);
        newSize = workload - (index + 1) + recvdSublistCount;
        temp = malloc(newSize * sizeof(int));
        int newArrIndex = 0;
        for(int i = index + 1; i < workload; i++) {
            temp[newArrIndex] = myarr[i];
            newArrIndex++;
        }
        for(int i = 0; i < recvdSublistCount; i++) {
            temp[newArrIndex] = recvd[i];
            newArrIndex++;
        }
        for(int i= 0; i < newSize; i++) {
            printf("id: %d, %dth element: %d \n", myid, i, temp[i]);
        }       
        free(recvd); 
    }

    free(myarr);
    myarr = temp;
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