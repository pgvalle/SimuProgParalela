// using more
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
    int rank, size;
    double result, result_local = 0;
    double *x, *y;
    double *x_local, *y_local;
    int n, part;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        assert(argc == 2);
        n = atoi(argv[1]);
        x = calloc(n, sizeof(*x));
        y = calloc(n, sizeof(*y));
        // inicializa vetor x e y
        for (int i = 0; i < n; i++) {
            x[i] = 1;
            y[i] = 2;
        }
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    printf("Eu sou p%d, n=%d\n", rank, n);

    part = n / size;
    x_local = calloc(part, sizeof(*x));
    y_local = calloc(part, sizeof(*y));

    MPI_Scatter(x, part, MPI_DOUBLE, x_local, part, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(y, part, MPI_DOUBLE, y_local, part, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    for (int i = 0; i < part; i++) {
        result_local += x_local[i] * y_local[i];
    }

    MPI_Reduce(&result_local, &result, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("result = %f\n", result);
    }
/*
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
*/
    free(x); free(y);
    free(x_local); free(y_local);

    MPI_Finalize();
    return 0;
}