#include <time.h>
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
    bool *adj; // adjacency matrix
} graph_t;

graph_t read_graph();
bool test_isomorphism(graph_t g1, graph_t g2);
bool check_perm(graph_t g1, graph_t g2, int *perm);
void print_perm(int *perm, int n);

int main() {
    graph_t g1 = read_graph();
    graph_t g2 = read_graph();

#if 1
    test_isomorphism(g1, g2);
#else
    if (test_isomorphism(g1, g2)) {
        printf("the graphs are isomorphic!\n");
    } else {
        printf("the graphs are not isomorphic!\n");
    }
#endif

    free(g1.adj);
    free(g2.adj);

    return 0;
}

graph_t read_graph() {
    graph_t g;

    scanf("%d %d", &g.n_verts, &g.n_edges);
    g.adj = malloc(g.n_verts * g.n_verts);

    for (int i = 0; i < g.n_edges; i++) {
        int s, d;
        scanf("%d %d", &s, &d);
        g.adj[s * g.n_verts + d] = true; 
        g.adj[d * g.n_verts + s] = true;
    }

    return g;
}

bool test_isomorphism(graph_t g1, graph_t g2) {
    if (g1.n_verts != g2.n_verts || g1.n_edges != g2.n_edges) {
        return false;
    }

    int n = g1.n_verts;
    int indices[n];
    bool isomorphism = false;

    for (int i = 0; i < n; i++) {
        indices[i] = i;
    }

    clock_t delta = clock();

    for (uint64_t i = 0; i < fat[n]; i++) {
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
            isomorphism = false;
            break;
        }
    }

    delta = clock() - delta;
    printf("%f\n", (float)delta / CLOCKS_PER_SEC);

    return isomorphism;
}

bool check_perm(graph_t g1, graph_t g2, int *perm) {
    int n = g1.n_verts;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if(g1.adj[i*n+j] != g2.adj[perm[i]*n+perm[j]]) {
                return false;
            }
        }
    }
    return true;
}

void print_perm(int *perm, int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", perm[i]);
    }
    printf("\n");
}
