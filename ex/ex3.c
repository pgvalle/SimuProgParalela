#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

int main(int argc, char** argv) {
    srand(time(NULL));
    int rank, size;
    int num;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        num = rand() % size;
        printf("initial number: %d\n", num);
        MPI_Send(&num, 1, MPI_INT, (rank+1) % size, 0, MPI_COMM_WORLD);
        MPI_Recv(&num, 1, MPI_INT, size - 1, 0, MPI_COMM_WORLD, &status);
        printf("final number: %d\n", num);
    } else {
        MPI_Status status;
        MPI_Recv(&num, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &status);
        printf("p%d received %d\n", rank, num);
        num += rank;
        printf("p%d sent %d\n", rank, num);
        MPI_Send(&num, 1, MPI_INT, (rank+1) % size, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}