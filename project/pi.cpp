#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>


using namespace std;

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: mpirun -np <thread_number> ./pi <point_number_per_thread>");
        exit(1);
    }

    long int point = atoi(argv[1]);

    MPI_Init(&argc, &argv);

    char hostname[MPI_MAX_PROCESSOR_NAME];
    int myid, idsize, inside = 0;
    int len;

    double x, y, hat_pi;

    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &idsize);
    MPI_Get_processor_name(hostname, &len);
    printf("Process %d of %d on %s\n",myid,idsize,hostname);
    //bcast point number
    if (myid == 0){
        for (int id = 1; id < idsize; id++){
            MPI_Send(&point, 1, MPI_INT, id, 0, MPI_COMM_WORLD);
        }

    } else{
        MPI_Recv(&point, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    float *rands = new float[2*point];
    srand((int)time(0));
    for (int i=0; i<2*point; i++){
        rands[i] = (rand() / (float)RAND_MAX);

    }

    for (int i=0;i<point;i++){
        x = rands[2*i];
        y = rands[2*i+1];
        if (x*x+y*y<1){
            inside++;
        }
    }

    delete[] rands;

    if (myid == 0){
        for (int id=1; id<idsize; id++){
            int temp;
            MPI_Recv(&temp,1,MPI_INT,id,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            inside += temp;
        }
    } else {
        MPI_Send(&inside, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    if (myid == 0){
        hat_pi = 4 * (double) inside / (double) (idsize * point);
        printf("Estimated Pi = %.10f \n",hat_pi);
    }

    MPI_Finalize();

    return 0;
}