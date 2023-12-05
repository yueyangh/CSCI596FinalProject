#include "dijkstra.h"
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

MPI_Datatype Build_blk_col_type(int n, int loc_n);
void Dijkstra_Init(int loc_mat[], int loc_pred[], int loc_dist[], int loc_known[], int my_rank, int loc_n);
void Dijkstra(int loc_mat[], int loc_dist[], int loc_pred[], int loc_n, int n, MPI_Comm comm);
int Find_min_dist(int loc_dist[], int loc_known[], int loc_n);
void Print_matrix(int global_mat[], int rows, int cols);
void Print_dists(int global_dist[], int n);
void Print_paths(int global_pred[], int n);


// perform dijkstra using mpi
void dijkstra_mpi(int *graph, int n, int my_rank, int p, double *time_taken) {
    int *loc_mat, *loc_dist, *loc_pred, *global_dist = NULL, *global_pred = NULL;
    int loc_n;
    MPI_Datatype blk_col_mpi_t;
    double start_time, end_time;

    loc_n = n / p;
    loc_mat = malloc(n * loc_n * sizeof(int));
    loc_dist = malloc(loc_n * sizeof(int));
    loc_pred = malloc(loc_n * sizeof(int));
    blk_col_mpi_t = Build_blk_col_type(n, loc_n);

    if (my_rank == 0) {
        global_dist = malloc(n * sizeof(int));
        global_pred = malloc(n * sizeof(int));
    } 

    MPI_Scatter(graph, 1, blk_col_mpi_t, loc_mat, n * loc_n, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();   

    Dijkstra(loc_mat, loc_dist, loc_pred, loc_n, n, MPI_COMM_WORLD);

    /* Gather the results from Dijkstra */
    MPI_Gather(loc_dist, loc_n, MPI_INT, global_dist, loc_n, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(loc_pred, loc_n, MPI_INT, global_pred, loc_n, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();

    /* Get time */
    if (my_rank == 0) {
        *time_taken = end_time - start_time;
        free(global_dist);
        free(global_pred);
    }
    free(loc_mat);
    free(loc_pred);
    free(loc_dist);
    MPI_Type_free(&blk_col_mpi_t);
}


// Build an MPI_Datatype that represents a block column of a matrix
MPI_Datatype Build_blk_col_type(int n, int loc_n) {
    MPI_Aint lb, extent;
    MPI_Datatype block_mpi_t;
    MPI_Datatype first_bc_mpi_t;
    MPI_Datatype blk_col_mpi_t;

    MPI_Type_contiguous(loc_n, MPI_INT, &block_mpi_t);
    MPI_Type_get_extent(block_mpi_t, &lb, &extent);

    /* MPI_Type_vector(numblocks, elts_per_block, stride, oldtype, *newtype) */
    MPI_Type_vector(n, loc_n, n, MPI_INT, &first_bc_mpi_t);

    /* This call is needed to get the right extent of the new datatype */
    MPI_Type_create_resized(first_bc_mpi_t, lb, extent, &blk_col_mpi_t);

    MPI_Type_commit(&blk_col_mpi_t);

    MPI_Type_free(&block_mpi_t);
    MPI_Type_free(&first_bc_mpi_t);

    return blk_col_mpi_t;
}


// Initialize all the matrices so that Dijkstras shortest path can be run
void Dijkstra_Init(int loc_mat[], int loc_pred[], int loc_dist[], int loc_known[],
                   int my_rank, int loc_n) {
    int loc_v;

    if (my_rank == 0)
        loc_known[0] = 1;
    else
        loc_known[0] = 0;

    for (loc_v = 1; loc_v < loc_n; loc_v++)
        loc_known[loc_v] = 0;

    for (loc_v = 0; loc_v < loc_n; loc_v++) {
        loc_dist[loc_v] = loc_mat[0 * loc_n + loc_v];
        loc_pred[loc_v] = 0;
    }
}

// compute all the shortest paths from the source vertex 0 to all vertices v
void Dijkstra(int loc_mat[], int loc_dist[], int loc_pred[], int loc_n, int n,
              MPI_Comm comm) {

    int i, loc_v, loc_u, glbl_u, new_dist, my_rank, dist_glbl_u;
    int *loc_known;
    int my_min[2];
    int glbl_min[2];

    MPI_Comm_rank(comm, &my_rank);
    loc_known = malloc(loc_n * sizeof(int));

    Dijkstra_Init(loc_mat, loc_pred, loc_dist, loc_known, my_rank, loc_n);

    for (i = 0; i < n - 1; i++) {
        loc_u = Find_min_dist(loc_dist, loc_known, loc_n);

        if (loc_u != -1) {
            my_min[0] = loc_dist[loc_u];
            my_min[1] = loc_u + my_rank * loc_n;
        }
        else {
            my_min[0] = INFINITY;
            my_min[1] = -1;
        }

        /* Get the minimum distance found by the processes and store that
           distance and the global vertex in glbl_min
        */
        MPI_Allreduce(my_min, glbl_min, 1, MPI_2INT, MPI_MINLOC, comm);

        dist_glbl_u = glbl_min[0];
        glbl_u = glbl_min[1];

        /* This test is to assure that loc_known is not accessed with -1 */
        if (glbl_u == -1)
            break;

        /* Check if global u belongs to process, and if so update loc_known */
        if ((glbl_u / loc_n) == my_rank) {
            loc_u = glbl_u % loc_n;
            loc_known[loc_u] = 1;
        }

        for (loc_v = 0; loc_v < loc_n; loc_v++) {
            if (!loc_known[loc_v]) {
                new_dist = dist_glbl_u + loc_mat[glbl_u * loc_n + loc_v];
                if (new_dist < loc_dist[loc_v]) {
                    loc_dist[loc_v] = new_dist;
                    loc_pred[loc_v] = glbl_u;
                }
            }
        }
    }
    free(loc_known);
}

// find the minimum local distance from the source to the assigned vertices of the process that calls the method
int Find_min_dist(int loc_dist[], int loc_known[], int loc_n) {
    int loc_u = -1, loc_v;
    int shortest_dist = INFINITY;

    for (loc_v = 0; loc_v < loc_n; loc_v++) {
        if (!loc_known[loc_v]) {
            if (loc_dist[loc_v] < shortest_dist) {
                shortest_dist = loc_dist[loc_v];
                loc_u = loc_v;
            }
        }
    }
    return loc_u;
}