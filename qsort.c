#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

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
    FILE *fileptr = fopen(argv[1], "r");
    if (fileptr == NULL) { 
            printf("\n The file could "
                "not be opened.\n"); 
            exit(1); 
    } 
    int * arr = NULL;
    int num;
    int size = 0;
    struct timeval start_time, end_time;
    double elapsed_time;


    while(fscanf(fileptr, "%d", &num ) == 1) {
        arr = realloc(arr, (size + 1) * sizeof(int));
        arr[size++] = num;
    }

    fclose(fileptr);

    gettimeofday(&start_time, NULL);
    
    quicksort(arr,0,size - 1);
    writeArray(arr, size, argv[2]);
    
    /* for (int i = 0; i < size; i++){
        printf("%d ", arr[i]);
    } */
    free(arr);
    gettimeofday(&end_time, NULL);
    elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                   (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    printf("Serial implementation lasted %f seconds.\n", elapsed_time);
    
    return 0;
}