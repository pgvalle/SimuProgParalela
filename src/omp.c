#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

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

typedef struct {
    int n_verts, n_edges;
    bool adj[400]; // adjacency matrix (max 20 vertices)
} graph_t;

void read_graph(graph_t *g);
bool check_isomorphism(const graph_t *g1, const graph_t *g2);
bool check_perm(const graph_t *g1, const graph_t *g2, const int *perm);

int main() {
    graph_t *ga = malloc(2 * sizeof(*ga));

    read_graph(&ga[0]);
    read_graph(&ga[1]);

#ifdef SCRIPT
    check_isomorphism(&ga[0], &ga[1]);
#else
    if (check_isomorphism(&ga[0], &ga[1])) {
        printf("the graphs are isomorphic!\n");
    } else {
        printf("the graphs are not isomorphic!\n");
    }
#endif

    free(ga);

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
    bool isomorphism = false;

    for (int i = 0; i < n; i++) {
        indices[i] = i;
    }

    double delta = omp_get_wtime();

    #pragma omp parallel shared(n, indices, isomorphism)
    { 
        bool local_isomorphism = false;
        
        #pragma omp for
        for (uint64_t i = 0; i < fat[n]; i++) {
            // skip iterations if isomorphism already found
            if (local_isomorphism) {
                continue;
            }

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
                local_isomorphism = true;
            }
        }

        #pragma omp critical
        if (local_isomorphism) {
            isomorphism = local_isomorphism;
        }
    }

    delta = omp_get_wtime() - delta;
    printf("%f\n", delta);

    return isomorphism;
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
