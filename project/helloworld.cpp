#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>


using namespace std;

int main(int argc, char *argv[]) {

    if (argc != 1) {
        printf("Usage: mpirun -np <thread_number> ./helloworld);
        exit(1);
    }



    MPI_Init(&argc, &argv);

    char hostname[MPI_MAX_PROCESSOR_NAME];
    int myid, idsize = 0;
    int len;

    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &idsize);
    MPI_Get_processor_name(hostname, &len);
    printf("Hello World from process %d of %d on %s\n",myid,idsize,hostname);

    MPI_Finalize();

    return 0;
}