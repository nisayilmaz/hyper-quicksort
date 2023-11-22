#include <stdio.h>

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
    printf("p: %d, r: %d", p, r);
    if(p < r) {
    
        int q = partition(arr, p, r);
        printf("pivot: %d", q);
        quicksort(arr, p, q);
        quicksort(arr, q + 1, r);
    }
}

int main() {
    int arr[] = {1,3,2,4,3,2};
    quicksort(arr,0,5);
    for (int i = 0; i < 7; i++)
    {
        printf("%d\n", arr[i]);
    }
    


    
    return 0;
}