#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  int n, p;
  double pi, pi_local, h, sum, x, a;
  int rank, size;
  MPI_Status status;


  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0) {
    n = atoi(argv[1]);
  }

  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  p = n / size;
  h = 1.0 / n;
  sum = 0.0;
  int end = (rank + 1) * p + ((rank == size - 1) ? (n % size) : 0);
  printf("start=%d end=%d\n", 1 + rank * p, end);

  for (int i = 1 + rank * p; i <= end; i++) {
    x = h * (i - 0.5);
    sum += 4.0 / (1.0 + x * x);
  }
  pi_local = h * sum;

  MPI_Reduce(&pi_local, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    printf("pi = %f\n", pi);
  }

  MPI_Finalize();

  return 0;
}