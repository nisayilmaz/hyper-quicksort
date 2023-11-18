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

int main() {
    int arr[] = {4,7,2,8,1,9,5};
    quicksort(arr,0,6);
    for (int i = 0; i < 7; i++)
    {
        printf("%d\n", arr[i]);
    }
    


    
    return 0;
}