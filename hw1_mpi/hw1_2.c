#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>

#define N 5 // change N to 1024 and run (print too many numbers)
#define ROOT 0
#define KERNEL_SIZE  4 // 4 x 4
#define STRIPE 1
#define MAX_RAND 10 //element of matrix is less than this

// Creates a matrix(row:m,col:n) of random numbers. Each number has a value from 0 ~ MAX_RAND
void rand_matrix(int matrix[][N]) {
    // Seed the random number
    //srand(time(NULL));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++){
            matrix[i][j] = rand() % MAX_RAND;
        }
    }
}


// print matrix of N * N
void print_matrix(int matrix[][N]){
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++){
            printf("%d ",matrix[i][j]);
        }
        printf("\n");
    }
}


// matrix&matrix multiplication function
void multiplication_matrix_matrix(int matrix1[][N], int matrix2[][N], int outmatrix[][N], MPI_Comm communicator){
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int block_row_num = (int)(N / world_size);
    // bcast right matrix
    MPI_Bcast(matrix2, N * N, MPI_INT, ROOT, communicator);
    // scatter rows of left matrix
    int rows_matrix1[block_row_num][N];
    MPI_Scatter(matrix1, N * block_row_num, MPI_INT, rows_matrix1, N * block_row_num, MPI_INT, ROOT, communicator);
    int rows_outmatrix[block_row_num][N];

    // multiplication
    int tempsum = 0;
    for (int z = 0; z < block_row_num; z++){
        for (int i = 0; i < N; i++){
            for (int j = 0; j < N; j++){
                tempsum += rows_matrix1[z][j] * matrix2[j][i];
            }
            //append to rows of outmatrix
            rows_outmatrix[z][i] = tempsum;
            tempsum = 0;
        }
    }

    // communication & compute left rows
    if (world_rank == ROOT){
        // receive and copy self
        for(int proc = 0; proc < world_size; proc++){
            if (proc != ROOT) {
                MPI_Recv(outmatrix[proc * block_row_num], N * block_row_num, MPI_INT, proc, 0, communicator,
                         MPI_STATUS_IGNORE);
            }
        }
        memcpy(outmatrix, rows_outmatrix, sizeof(int) * N * block_row_num);

        // compute left rows
        int left_row_num;
        left_row_num = N - block_row_num * world_size;
        if (left_row_num != 0){
            int left_outmatrix[left_row_num][N];
            int tempsum = 0;
            for (int z = 0; z < left_row_num; z++){
                for (int i = 0; i < N; i++){
                    for (int j = 0; j < N; j++){
                        tempsum += matrix1[N - left_row_num + z][j] * matrix2[j][i];
                    }
                    //append to rows of outmatrix
                    left_outmatrix[z][i] = tempsum;

                    tempsum = 0;
                }
            }
            memcpy(outmatrix[block_row_num * world_size], left_outmatrix, sizeof(int) * N * left_row_num);
        }
    }else{
        MPI_Send(rows_outmatrix[0],N * block_row_num, MPI_INT, ROOT, 0, communicator);
    }
}


int get_sub_matrix_max(int matrix[][N],int start_i, int start_j,int kernel){
    int max_temp;
    max_temp = matrix[0][0];
    for (int i = start_i; i < start_i + kernel ;i++){
        for (int j = start_j; j < start_j + kernel ;j++){
            if (matrix[i][j] > max_temp){
                max_temp = matrix[i][j];
            }
        }
    }
    return max_temp;
}


void max_pooling(int matrix1[][N], int out_matrix[][(N-KERNEL_SIZE+1)/STRIPE],int kernel, int stripe, MPI_Comm communicator){
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int run_row_n = (int)((N - kernel + 1) / stripe); // run col number = run row number
    int blockn = (int)(run_row_n / world_size);// run col(row) number of every proccessor
    int rest_run_row_num = run_row_n - blockn * world_size ;


    // bcast matrix, divide rows
    MPI_Bcast(matrix1, N * N, MPI_INT, ROOT, communicator);

    int *maxbuff = (int *)malloc(sizeof(int) * blockn * run_row_n);
    for (int i = world_rank*stripe;i< stripe*(world_rank + blockn);i+=stripe){
        for (int j = 0;j < N - kernel + 1; j+=stripe){
             maxbuff[((i-world_rank*stripe)/stripe)*run_row_n+j/stripe] = get_sub_matrix_max(matrix1,i,j,KERNEL_SIZE);
        }
    }

    //printf("{%d}{%d} ",maxbuff[0],maxbuff[1]);
    if (world_rank != ROOT) {
        MPI_Send(maxbuff, blockn * run_row_n, MPI_INT, ROOT, 0, communicator);
    } else {
        //printf("==%d==%d==", blockn, run_row_n);
        for (int proc = 0; proc < world_size; proc++) {
            if (proc != ROOT) {
                MPI_Recv(out_matrix[blockn*proc], blockn * run_row_n, MPI_INT, proc, 0, communicator, MPI_STATUS_IGNORE);
            }
        }
        memcpy(out_matrix[0], maxbuff, sizeof(int) * blockn * run_row_n);
    }

        //rest
    if (world_rank == ROOT && rest_run_row_num !=0) {
        for (int i = stripe*run_row_n;i< N-kernel+1;i+=stripe){
            for (int j = 0;j < N - kernel + 1; j+=stripe){
                out_matrix[i/stripe][j/stripe] =  get_sub_matrix_max(matrix1,i,j,KERNEL_SIZE);
            }
        }
    }

}


int get_sub_matrix_mult_weight_sum(int matrix[][N],int weight_matrix[][KERNEL_SIZE],int start_i, int start_j,int kernel){
    int sum_temp = 0;
    for (int i = start_i; i < start_i + kernel ;i++){
        for (int j = start_j; j < start_j + kernel ;j++){
                sum_temp += matrix[i][j] * weight_matrix[i][j];
        }
    }
    return sum_temp;
}


void convolution(int matrix1[][N],int weight[][KERNEL_SIZE], int out_matrix[][(N-KERNEL_SIZE+1)/STRIPE],int kernel, int stripe, MPI_Comm communicator){
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int run_row_n = (int)((N - kernel + 1) / stripe); // run col number = run row number
    int blockn = (int)(run_row_n / world_size);// run col(row) number of every proccessor
    int rest_run_row_num = run_row_n - blockn * world_size ;


    // bcast matrix, divide rows
    MPI_Bcast(matrix1, N * N, MPI_INT, ROOT, communicator);

    int *maxbuff = (int *)malloc(sizeof(int) * blockn * run_row_n);
    for (int i = world_rank*stripe;i< stripe*(world_rank + blockn);i+=stripe){
        for (int j = 0;j < N - kernel + 1; j+=stripe){
            maxbuff[((i-world_rank*stripe)/stripe)*run_row_n+j/stripe] = get_sub_matrix_mult_weight_sum(matrix1,weight,i,j,KERNEL_SIZE);
        }
    }

    //printf("{%d}{%d} ",maxbuff[0],maxbuff[1]);
    if (world_rank != ROOT) {
        MPI_Send(maxbuff, blockn * run_row_n, MPI_INT, ROOT, 0, communicator);
    } else {
        //printf("==%d==%d==", blockn, run_row_n);
        for (int proc = 0; proc < world_size; proc++) {
            if (proc != ROOT) {
                MPI_Recv(out_matrix[blockn*proc], blockn * run_row_n, MPI_INT, proc, 0, communicator, MPI_STATUS_IGNORE);
            }
        }
        memcpy(out_matrix[0], maxbuff, sizeof(int) * blockn * run_row_n);
    }

    //rest
    if (world_rank == ROOT && rest_run_row_num !=0) {
        for (int i = stripe*run_row_n;i< N-kernel+1;i+=stripe){
            for (int j = 0;j < N - kernel + 1; j+=stripe){
                out_matrix[i/stripe][j/stripe] =  get_sub_matrix_max(matrix1,i,j,KERNEL_SIZE);
            }
        }
    }

}


int main(int argc, char *argv[]){
    MPI_Init(NULL, NULL);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);


    int m1[N][N];
    int m2[N][N];

    if (world_rank == ROOT){
        rand_matrix(m1);
        rand_matrix(m2);
    }

    // 2.1 Mulitplication
    int m3[N][N]; // m1 * m2    --->> m3
    multiplication_matrix_matrix(m1, m2, m3, MPI_COMM_WORLD);
    if (world_rank == ROOT){
        printf("Matrix 1 is:\n");
        print_matrix(m1);
        printf("Matrix 2 is:\n");
        print_matrix(m2);
        printf("Matrix 1 * Matrix 2 is:\n");
        print_matrix(m3);
    }

    // 2.2 max pooling
    int m4[(N+1-KERNEL_SIZE)/STRIPE][(N+1-KERNEL_SIZE)/STRIPE]; // m1 pooling -->> m4
    if (world_rank == ROOT){
        rand_matrix(m1);
        printf("Matrix 1 is:\n");
        print_matrix(m1);
    }
    max_pooling(m1,m4,KERNEL_SIZE,STRIPE,MPI_COMM_WORLD);
    if (world_rank == ROOT){
        printf("Matrix 1 after pooling is:\n");
        for (int i = 0; i < (N+1-KERNEL_SIZE)/STRIPE; i++) {
            for (int j = 0; j < (N + 1 - KERNEL_SIZE) / STRIPE; j++) {
                printf("%d ", m4[i][j]);
            }
            printf("\n");
        }
    }

    // 2.3 convolution
    int w[KERNEL_SIZE][KERNEL_SIZE]={{0,1,0,-1},{1,0,0,-1},{-1,0,1,0},{1,-1,-1,0}}; //weight
    int m5[(N+1-KERNEL_SIZE)/STRIPE][(N+1-KERNEL_SIZE)/STRIPE];    // m1 convolution -->> m5
    convolution(m1,w,m5,KERNEL_SIZE,STRIPE,MPI_COMM_WORLD);
    if (world_rank == ROOT){
        printf("Weight is:\n");
        for (int i = 0; i < KERNEL_SIZE; i++) {
            for (int j = 0; j < KERNEL_SIZE; j++) {
                printf("%d ", w[i][j]);
            }
            printf("\n");
        }
        printf("Matrix 1 after convolution is:\n");
        for (int i = 0; i < (N+1-KERNEL_SIZE)/STRIPE; i++) {
            for (int j = 0; j < (N + 1 - KERNEL_SIZE) / STRIPE; j++) {
                printf("%d ", m5[i][j]);
            }
            printf("\n");
        }
    }

    MPI_Finalize();
}