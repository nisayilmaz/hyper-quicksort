#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


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

void writeArray(int * arr, int size, char filename[30]){
    FILE * fileptr = fopen(filename, "w");
    for(int i = 0; i < size; i++){
        fprintf(fileptr, "%d", arr[i]);
        if (i != size - 1){
            fprintf(fileptr, "\n");        
        }
        
    } 
    fclose(fileptr);
}

int main(int argc, char* argv[]) {
    
    int dimension = log2(atoi(argv[2])); 

    int myid, numprocs, totalNum, myoriginalid;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm curr_comm = MPI_COMM_WORLD, new_comm;
    MPI_Status status;
    int * arr;
    int size;
    myoriginalid = myid;
    totalNum = numprocs;
    double start, end;

    if(myid == 0) {
        FILE *fileptr = fopen(argv[1], "r");
        if (fileptr == NULL) { 
            printf("\n The file could "
                "not be opened.\n"); 
            exit(1); 
        } 
  
        arr = NULL;
        int num;
        size = 0;

        while(fscanf(fileptr, "%d", &num ) == 1) {
            arr = realloc(arr, (size + 1) * sizeof(int));
            arr[size++] = num;
        }
        fclose(fileptr);
    }

    start = MPI_Wtime();
    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int workload = size / numprocs;
    int * myarr = malloc(workload * sizeof(int));
    MPI_Scatter(arr, workload, MPI_INT, myarr, workload, MPI_INT, 0, MPI_COMM_WORLD);
    int * medians;
    int mask, myPartner, maskedId, color, sublistCount, index, recvdSublistCount, newSize, key, medianOfMedians,median;

    int * recvd = NULL;
    int * temp = NULL;

    quicksort(myarr, 0, workload - 1); 
    while(numprocs > 1){
        if( workload % 2 != 0) {
            median = myarr[(workload - 1) / 2];
        }
        else {
            median = (myarr[workload/2] + myarr[(workload / 2) - 1]) / 2;
        }

        medians = malloc(numprocs * sizeof(int));
        MPI_Allgather(&median, 1, MPI_INT, medians, 1, MPI_INT, curr_comm);
        quicksort(medians, 0, numprocs - 1);
        medianOfMedians = (medians[numprocs/2] + medians[(numprocs / 2) - 1]) / 2;
        
        mask = pow(2, dimension - 1);
        myPartner = myid ^ mask;
        maskedId = myid & mask;
        sublistCount = 0;
        recvdSublistCount = 0;
        key = myid;
        recvd = NULL;
        temp = NULL;
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

            MPI_Send(&sublistCount, 1, MPI_INT, myPartner, 0, curr_comm);
            MPI_Send(&myarr[index], sublistCount, MPI_INT, myPartner, 1, curr_comm);
            MPI_Recv(&recvdSublistCount, 1, MPI_INT, myPartner, 2, curr_comm, &status);
            int * recvd = malloc(recvdSublistCount * sizeof(int));
            MPI_Recv(&recvd[0], recvdSublistCount, MPI_INT, myPartner, 3, curr_comm, &status);

            newSize = index + recvdSublistCount;
            temp = malloc(newSize * sizeof(int));
            int newArrIndex = 0;
            int myArrIndex = 0, recvdArrayIndex = 0;
            while(myArrIndex < index && recvdArrayIndex < recvdSublistCount){
                if(myarr[myArrIndex] <= recvd[recvdArrayIndex] ){
                    temp[newArrIndex] = myarr[myArrIndex];
                    myArrIndex++;
                    newArrIndex++;
                }
                else{
                    temp[newArrIndex] = recvd[recvdArrayIndex];
                    recvdArrayIndex++;
                    newArrIndex++;
                }
            }
            while (myArrIndex < index){
                temp[newArrIndex] = myarr[myArrIndex];
                myArrIndex++;
                newArrIndex++;
            }
            while(recvdArrayIndex < recvdSublistCount){
                temp[newArrIndex] = recvd[recvdArrayIndex];
                recvdArrayIndex++;
                newArrIndex++;
            }
          
            free(recvd);

        }
        else {
            color = 1;
            index = -1;
            MPI_Recv(&recvdSublistCount, 1, MPI_INT, myPartner, 0, curr_comm, &status);
            recvd = malloc(recvdSublistCount * sizeof(int));
            MPI_Recv(&recvd[0], recvdSublistCount, MPI_INT, myPartner, 1, curr_comm, &status);

            for(int i = 0; i < workload; i++){
                if(myarr[i] < medianOfMedians){
                    sublistCount++; 
                    index = i;
                }
                else {
                    break;
                }
            }       
            MPI_Send(&sublistCount, 1, MPI_INT, myPartner, 2, curr_comm);
            MPI_Send(&myarr[0], sublistCount, MPI_INT, myPartner, 3, curr_comm);
            newSize = workload - (index + 1) + recvdSublistCount;
            temp = malloc(newSize * sizeof(int));
            int newArrIndex = 0;
            int myArrIndex = index + 1, recvdArrayIndex = 0;

            while(myArrIndex < workload && recvdArrayIndex < recvdSublistCount){
                if(myarr[myArrIndex] <= recvd[recvdArrayIndex] ){
                    temp[newArrIndex] = myarr[myArrIndex];
                    myArrIndex++;
                    newArrIndex++;
                }
                else{
                    temp[newArrIndex] = recvd[recvdArrayIndex];
                    recvdArrayIndex++;
                    newArrIndex++;
                }
            }
            while (myArrIndex < workload){
                temp[newArrIndex] = myarr[myArrIndex];
                myArrIndex++;
                newArrIndex++;
            }
            while(recvdArrayIndex < recvdSublistCount){
                temp[newArrIndex] = recvd[recvdArrayIndex];
                recvdArrayIndex++;
                newArrIndex++;
            }

            free(recvd); 
        }

        free(myarr);
        myarr = temp;
        MPI_Comm_split(curr_comm, color, key, &new_comm);
        if(curr_comm != MPI_COMM_WORLD){
            MPI_Comm_free(&curr_comm);
        }

        curr_comm = new_comm;

        MPI_Comm_size(curr_comm, &numprocs);
        MPI_Comm_rank(curr_comm, &myid);
        dimension--;
        workload = newSize;
        free(medians);
    }
    char idStr[10];
    sprintf(idStr, "%d.txt", myoriginalid);
    char outName[20] = "output";
    strcat(outName, idStr);

    writeArray(myarr, workload, outName);
    int *recv_counts = malloc(totalNum * sizeof(int));
    MPI_Gather(&workload, 1, MPI_INT, recv_counts, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int *displacements = NULL;
    if(myoriginalid == 0) {
        displacements = malloc(totalNum * sizeof(int));
        displacements[0] = 0;
        for (int i = 1; i < totalNum; ++i) {
            displacements[i] = displacements[i - 1] + recv_counts[i - 1] ;
        }
    }
    MPI_Gatherv(myarr, workload, MPI_INT, arr, recv_counts, displacements, MPI_INT, 0, MPI_COMM_WORLD);

    if(myoriginalid == 0) {
        end = MPI_Wtime();
        printf("Parallel implementation with %d processes lasted %f seconds.\n", totalNum, end - start);
        /* for(int i = 0; i < size; i++) {
            printf("%d ", arr[i]);
        } */
        writeArray(arr, size, argv[3]);
        free(arr);

    }
    MPI_Comm_free(&curr_comm);
    free(recv_counts);
    free(displacements);
    free(myarr);
    MPI_Finalize();


    return 0;
}