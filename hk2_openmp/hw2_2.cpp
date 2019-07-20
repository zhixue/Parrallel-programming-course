#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#define NUM_COUNT 1000000

/* quick sort 1000000 number  */
void random_num(int *rand_nums, int count){
    for (int i = 0; i < count; i++) {
        rand_nums[i] = (int)rand();
    }
}


void swap(int &a, int &b)//
{
    int temp;
    temp = a;
    a = b;
    b = temp;
}


void quicksort(int *num,int low,int high){
    if(low < high)
    {
        int split = low;
        for (int i=low+1;i<high;i++) {
            if (num[i] < num[low])
                swap(num[++split], num[i]);
        }
        swap(num[low], num[split]);
#pragma omp parallel sections
        {
#pragma omp section
            quicksort(num,low,split-1);
#pragma omp section
            quicksort(num,split+1,high);
        }
    }

}


int main(int argc,char** argv){
    if (argc != 2) {
        fprintf(stderr, "Usage: ./hw2_2 thread_number\n");
        exit(1);
    }
    int thread_count;
    thread_count=strtol(argv[1],NULL,10);

    int num_list[NUM_COUNT];
    random_num(num_list,NUM_COUNT);
    printf("Raw numbers:\n");
    for(int k=0;k<NUM_COUNT;k++){
        printf("%d ",num_list[k]);
    }
    printf("\n");

    // sort
    quicksort(num_list,0,NUM_COUNT);

    printf("Sorted numbers:\n");
    for(int k=0;k<NUM_COUNT;k++){
        printf("%d ",num_list[k]);
    }
    printf("\n");

}