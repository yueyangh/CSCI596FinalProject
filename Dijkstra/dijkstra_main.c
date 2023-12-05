#include "dijkstra.h"
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// randomly generate a graph with n vertices (stored as a 1D array)
void Generate_random_matrix(int *mat, int n) {
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            mat[i * n + j] = INFINITY;
        }
    }

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (i == j) {
                mat[i * n + j] = 0; 
                mat[j * n + i] = 0;
            } else if (mat[i * n + j] == INFINITY && mat[j * n + i] == INFINITY) {
                int randChoice = rand() % 3;
                if (randChoice == 0) {
                    mat[i * n + j] = (rand() % MAX_WEIGHT) + 1;
                } else if (randChoice == 1) {
                    mat[j * n + i] = (rand() % MAX_WEIGHT) + 1;
                }
            }
        }
    }
}

// run and compare serial and mpi version of Dijkstra's
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <number of vertices>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);  
    if (n <= 0) {
        fprintf(stderr, "Number of vertices must be a positive integer\n");
        exit(EXIT_FAILURE);
    }

    MPI_Init(&argc, &argv);
    int my_rank, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    int *graph = malloc(n * n * sizeof(int));
    double serial_time_taken;
    double mpi_time_taken;

    if (my_rank == 0) {
        Generate_random_matrix(graph, n);

        dijkstra_serial(graph, n, 0, &serial_time_taken);
        printf("Serial Dijkstra Execution Time(s): %f\n", serial_time_taken);
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD); // broadcast n to the other processes

    dijkstra_mpi(graph, n, my_rank, p, &mpi_time_taken);

    if (my_rank == 0) {
        printf("Parallel Dijkstra Execution Time(s): %f \n", mpi_time_taken);
    }
    MPI_Finalize();

    free(graph);
    return 0;
}