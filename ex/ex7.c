#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

int main(int argc, char** argv) {
    srand(time(NULL));
    int rank, size;
    struct { int len, k; } params;
    double* vec;
    double start, delta;
    MPI_Status stat;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        assert(argc == 3);
        params.len = atoi(argv[1]);
        params.k = atoi(argv[2]);
        // allocate vec
        vec = calloc(params.len, sizeof(*vec));
        // initialize
        for (int i = 0; i < params.len; i++) {
            vec[i] = 3;
        }
        // send params
        MPI_Send(&params, sizeof(params), MPI_BYTE, 1, 0, MPI_COMM_WORLD);
        // measure time
        start = MPI_Wtime();
    } else {
        // receive params
        MPI_Recv(&params, sizeof(params), MPI_BYTE, 0, 0, MPI_COMM_WORLD, &stat);
        // allocate vec
        vec = calloc(params.len, sizeof(*vec));
    }

    for (int i = 0; i < params.k; i++) {
        if (rank == 0) {
            MPI_Send(vec, params.len, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(vec, params.len, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, &stat);
        } else {
            MPI_Recv(vec, params.len, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &stat);
            MPI_Send(vec, params.len, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
    }

    if (rank == 0) {
        delta = MPI_Wtime() - start;
        printf("total=%f, per_msg=%f\n", delta, delta / (2 * params.k));
    }
    

    MPI_Finalize();
    return 0;
}