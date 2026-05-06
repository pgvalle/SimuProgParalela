#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

static const uint64_t fat[21] = {
    1,
    1,
    2,
    6,
    24,
    120,
    720,
    5040,
    40320,
    362880,
    3628800,
    39916800,
    479001600,
    6227020800,
    87178291200,
    1307674368000,
    20922789888000,
    355687428096000,
    6402373705728000,
    121645100408832000,
    2432902008176640000
};

int rank, size;

typedef struct {
    int n_verts, n_edges;
    bool adj[400]; // adjacency matrix (max 20 vertices)
} graph_t;

void read_graph(graph_t *g);
bool check_isomorphism(const graph_t *g1, const graph_t *g2);
bool check_perm(const graph_t *g1, const graph_t *g2, const int *perm);

int main(int argc, char** argv) {
    graph_t *ga = malloc(2 * sizeof(*ga));

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // register new type

    int blockcounts[3] = {1, 1, 400};
    MPI_Aint offsets[3] = {
        offsetof(graph_t, n_verts),
        offsetof(graph_t, n_edges),
        offsetof(graph_t, adj)
    };
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_C_BOOL};
    MPI_Datatype mpi_graph;

    MPI_Type_create_struct(3, blockcounts, offsets, types, &mpi_graph);
    MPI_Type_commit(&mpi_graph);

    // only rank 0 will do this
    if (rank == 0) {
        read_graph(&ga[0]);
        read_graph(&ga[1]);
        MPI_Bcast(ga, 2, mpi_graph, 0, MPI_COMM_WORLD);
        printf("p0 sent the graphs to everyone\n");
    } else { // other ranks receive
        MPI_Bcast(ga, 2, mpi_graph, 0, MPI_COMM_WORLD);
        printf("p%d received the graphs\n", rank);
    }

    // everyone checks
    bool iso = check_isomorphism(&ga[0], &ga[1]);
    printf("p%d says: %sisomorphic\n", rank, (iso ? "" : "not "));

    // rank 0 aggregates
    if (rank == 0) {
        // recv other ranks messages
        // do the check and report result
    } else { // other ranks send their results
        // send iso
        // MPI_Send(&iso, 1, MPI_C_BOOL, 0, 0, MPI_COMM_WORLD);
    }

    free(ga);

    MPI_Finalize();

    return 0;
}

void read_graph(graph_t *g) {
    scanf("%d %d", &g->n_verts, &g->n_edges);
    memset(g->adj, 0, sizeof(g->adj));

    for (int i = 0; i < g->n_edges; i++) {
        int s, d;
        scanf("%d %d", &s, &d);
        g->adj[s * g->n_verts + d] = true; 
        g->adj[d * g->n_verts + s] = true;
    }
}

bool check_isomorphism(const graph_t *g1, const graph_t *g2) {
    if (g1->n_verts != g2->n_verts || g1->n_edges != g2->n_edges) {
        return false;
    }

    int n = g1->n_verts;
    int indices[n];

    for (int i = 0; i < n; i++) {
        indices[i] = i;
    }

    uint64_t range = fat[n] / size;
    uint64_t start = rank * range;
    uint64_t end = (rank + 1) * range;

    if (rank == size - 1) {
        end += fat[n] % size;
    }

    for (uint64_t i = start; i < end; i++) {
        uint64_t factoradic[n];
        uint64_t v = i;
        int perm[n], local_indices[n];

        memcpy(local_indices, indices, sizeof(indices));

        for (int j = 0; j < n; j++) {
            uint64_t q = v / fat[n-j-1];

            v %= fat[n-j-1];
            factoradic[j] = q;
            perm[j] = local_indices[q];

            // remove the local index used
            for (int k = q; k < n-1; k++) {
                local_indices[k] = local_indices[k+1];
            }
        }

        if (check_perm(g1, g2, perm)) {
            return true;
        }
    }

    return false;
}

bool check_perm(const graph_t *g1, const graph_t *g2, const int *perm) {
    int n = g1->n_verts;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if(g1->adj[i*n+j] != g2->adj[perm[i]*n+perm[j]]) {
                return false;
            }
        }
    }

    return true;
}
