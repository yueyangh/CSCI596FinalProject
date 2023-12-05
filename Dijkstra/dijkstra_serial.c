#include "dijkstra.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int minDistance(int dist[], int sptSet[], int n) {
    int min = INFINITY, min_index;
    for (int v = 0; v < n; v++) {
        if (sptSet[v] == 0 && dist[v] <= min) {
            min = dist[v], min_index = v;
        }
    }
    return min_index;
}

// perform dijkstra serially
void dijkstra_serial(int *graph, int n, int src, double *time_taken) {

    int dist[n]; 
    int sptSet[n]; 

    for (int i = 0; i < n; i++) {
        dist[i] = INFINITY, sptSet[i] = 0;
    }

    clock_t start = clock();

    dist[src] = 0;

    for (int count = 0; count < n - 1; count++) {
        int u = minDistance(dist, sptSet, n);

        sptSet[u] = 1;

        for (int v = 0; v < n; v++) {
            if (!sptSet[v] && graph[u * n + v] && dist[u] != INFINITY && dist[u] + graph[u * n + v] < dist[v]) {
                dist[v] = dist[u] + graph[u * n + v];
            }
        }
    }

    clock_t end = clock();
    *time_taken = ((double)end - start) / CLOCKS_PER_SEC;

    /* print out results
    printf("Vertex\tDistance from Source\n");
    for (int i = 0; i < n; i++) {
        printf("%d \t %d\n", i, dist[i]);
    } */
}