#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

/* use Monte Carlo to estimate pi(near 3.1415926) */

int main(int argc,char** argv){
    if (argc != 2) {
        fprintf(stderr, "Usage: ./hw2_1 thread_number\n");
        exit(1);
    }

    int thread_count;
    thread_count=strtol(argv[1],NULL,10);

    // init
    long int point_num = 2000000000; // change here
    long int cycle_num = 0;
    printf("Use %d points to compute PI\n",point_num);

    srand(time(NULL));
    double x,y,distance_point;
    int i;

    // start
    //clock_t start;
    //start = clock();
#pragma omp parallel for num_threads(thread_count) \
        default(none) reduction(+:cycle_num) private(i,x,y,distance_point)\
        shared(point_num)
    for(i = 0; i < point_num; i++){
        // generate x,y
        x = (double)rand()/(double)RAND_MAX;
        y = (double)rand()/(double)RAND_MAX;

        // equal to sqtr(distance) < 1
        distance_point = x * x + y * y;
        if(distance_point<1){
            cycle_num++;
        }
    }

    // compute pi
    double estimate_pi=(double)cycle_num / point_num * 4;
    //printf("time used: %f seconds\n",(double)(clock()-start)/CLOCKS_PER_SEC);
    printf("the estimated value of pi is %f\n",estimate_pi);
    return 0;
}