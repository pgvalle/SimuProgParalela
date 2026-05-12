#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define N 52
double x[N], y[N];

int main(int argc, char** argv) {
    int rank, size;
    double result = 0;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int part = N / size;
    int ppart = part + ((rank == size - 1) ? (N % size) : 0);

    if (rank == 0) {
        // inicializa vetor x e y
        for (int i = 0; i < N; i++) {
            x[i] = 1;
            y[i] = 2;
        }
        // send
        for (int i = 1; i < size; i++) {
            MPI_Send(x + i*part, ppart, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            MPI_Send(y + i*part, ppart, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }
    } else {
        // receive
        MPI_Recv(x + rank*part, ppart, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(y + rank*part, ppart, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
    }

    // dot product
    printf("%d %d %d %d\n", rank, part, ppart, size);
    for (int i = rank * part; i < rank * part + ppart; i++) {
        result += x[i] * y[i];
    }

    // receive if 0 otherwise send
    if (rank == 0) {
        for (int i = 1; i < size; i++) {
            double other;
            MPI_Recv(&other, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            result += other;
            // printf("result from p%d %d\n", status, other);
        }
        printf("%f\n", result);
    } else {
        MPI_Send(&result, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&result, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}