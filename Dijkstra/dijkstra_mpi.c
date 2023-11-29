#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define INFINITY 1000000
#define MAX_WEIGHT 20

int Get_n(int my_rank, MPI_Comm comm);
MPI_Datatype Build_blk_col_type(int n, int loc_n);
void Read_matrix(int loc_mat[], int n, int loc_n, MPI_Datatype blk_col_mpi_t,
                 int my_rank, MPI_Comm comm);
void Dijkstra_Init(int loc_mat[], int loc_pred[], int loc_dist[], int loc_known[],
                   int my_rank, int loc_n);
void Dijkstra(int loc_mat[], int loc_dist[], int loc_pred[], int loc_n, int n,
              MPI_Comm comm);
int Find_min_dist(int loc_dist[], int loc_known[], int loc_n);
void Print_matrix(int global_mat[], int rows, int cols);
void Print_dists(int global_dist[], int n);
void Print_paths(int global_pred[], int n);

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
                mat[i * n + j] = 0; // 从顶点到其自身的距离为0
                mat[j * n + i] = 0;
            } else if (mat[i * n + j] == INFINITY && mat[j * n + i] == INFINITY) {
                int randChoice = rand() % 3;
                if (randChoice == 0) {
                    // 创建从i到j的边
                    mat[i * n + j] = (rand() % MAX_WEIGHT) + 1;
                } else if (randChoice == 1) {
                    // 创建从j到i的边
                    mat[j * n + i] = (rand() % MAX_WEIGHT) + 1;
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    int *loc_mat, *loc_dist, *loc_pred, *global_dist = NULL, *global_pred = NULL;
    int my_rank, p, loc_n, n;
    MPI_Comm comm;
    MPI_Datatype blk_col_mpi_t;
    double start_time, end_time;

    MPI_Init(NULL, NULL);
    comm = MPI_COMM_WORLD;
    MPI_Comm_rank(comm, &my_rank);
    MPI_Comm_size(comm, &p);
    n = Get_n(my_rank, comm);
    loc_n = n / p;
    loc_mat = malloc(n * loc_n * sizeof(int));
    loc_dist = malloc(loc_n * sizeof(int));
    loc_pred = malloc(loc_n * sizeof(int));
    blk_col_mpi_t = Build_blk_col_type(n, loc_n);

    int *mat = NULL;

    if (my_rank == 0) {
        global_dist = malloc(n * sizeof(int));
        global_pred = malloc(n * sizeof(int));
        mat = malloc(n * n * sizeof(int));
        Generate_random_matrix(mat, n);
        //Print_matrix(mat, n, n);
    } 
    MPI_Scatter(mat, 1, blk_col_mpi_t, loc_mat, n * loc_n, MPI_INT, 0, comm);
    if (my_rank == 0) {
        free(mat); // 释放主进程的内存
    }

    // 同步所有进程并开始计时
    MPI_Barrier(comm);
    start_time = MPI_Wtime();   

    //Read_matrix(loc_mat, n, loc_n, blk_col_mpi_t, my_rank, comm);
    Dijkstra(loc_mat, loc_dist, loc_pred, loc_n, n, comm);

    /* Gather the results from Dijkstra */
    MPI_Gather(loc_dist, loc_n, MPI_INT, global_dist, loc_n, MPI_INT, 0, comm);
    MPI_Gather(loc_pred, loc_n, MPI_INT, global_pred, loc_n, MPI_INT, 0, comm);

    MPI_Barrier(comm);
    end_time = MPI_Wtime();

    /* Print results */
    if (my_rank == 0) {
        //Print_dists(global_dist, n);
        //Print_paths(global_pred, n);
        printf("Process %d: Dijkstra algorithm took %f seconds to execute\n", my_rank, end_time - start_time);
        free(global_dist);
        free(global_pred);
    }
    free(loc_mat);
    free(loc_pred);
    free(loc_dist);
    MPI_Type_free(&blk_col_mpi_t);
    MPI_Finalize();
    return 0;
}


/*---------------------------------------------------------------------
 * Get the number of vertices in the graph on process 0  
 * and broadcast this value to the other processes
 */
int Get_n(int my_rank, MPI_Comm comm) {
    int n;

    if (my_rank == 0) {
        n = 10000;
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, comm);
    return n;
}

/*---------------------------------------------------------------------
 * Function:  Build_blk_col_type
 * Purpose:   Build an MPI_Datatype that represents a block column of
 *            a matrix
 * In args:   n:  number of rows in the matrix and the block column
 *            loc_n = n/p:  number cols in the block column
 * Ret val:   blk_col_mpi_t:  MPI_Datatype that represents a block
 *            column
 */
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


/*---------------------------------------------------------------------
 * Function:  Read_matrix
 * Purpose:   Read in an nxn matrix of ints on process 0, and
 *            distribute it among the processes so that each
 *            process gets a block column with n rows and n/p
 *            columns
 * In args:   n:  the number of rows/cols in the matrix and the submatrices
 *            loc_n = n/p:  the number of columns in the submatrices
 *            blk_col_mpi_t:  the MPI_Datatype used on process 0
 *            my_rank:  the caller's rank in comm
 *            comm:  Communicator consisting of all the processes
 * Out arg:   loc_mat:  the calling process' submatrix (needs to be
 *               allocated by the caller)
 */
void Read_matrix(int loc_mat[], int n, int loc_n,
                 MPI_Datatype blk_col_mpi_t, int my_rank, MPI_Comm comm) {
    int *mat = NULL, i, j;

    if (my_rank == 0) {
        mat = malloc(n * n * sizeof(int));
        for (i = 0; i < n; i++)
            for (j = 0; j < n; j++)
                scanf("%d", &mat[i * n + j]);
    }

    MPI_Scatter(mat, 1, blk_col_mpi_t, loc_mat, n * loc_n, MPI_INT, 0, comm);

    if (my_rank == 0) free(mat);
}


/*-------------------------------------------------------------------
 * Function:   Dijkstra_Init
 * Purpose:    Initialize all the matrices so that Dijkstras shortest path
 *             can be run
 *
 * In args:    loc_n:    local number of vertices
 *             my_rank:  the process rank
 *
 * Out args:   loc_mat:  local matrix containing edge costs between vertices
 *             loc_dist: loc_dist[v] = shortest distance from the source to each vertex v
 *             loc_pred: loc_pred[v] = predecessor of v on a shortest path from source to v
 *             loc_known: loc_known[v] = 1 if vertex has been visited, 0 else
 *
 *
 */
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

/*-------------------------------------------------------------------
 * Function:   Dijkstra
 * Purpose:    compute all the shortest paths from the source vertex 0
 *             to all vertices v
 *
 *
 * In args:    loc_mat:  local matrix containing edge costs between vertices
 *             loc_n:    local number of vertices
 *             n:        total number of vertices (globally)
 *             comm:     the communicator
 *
 * Out args:   loc_dist: loc_dist[v] = shortest distance from the source to each vertex v
 *             loc_pred: loc_pred[v] = predecessor of v on a shortest path from source to v
 *
 */
void Dijkstra(int loc_mat[], int loc_dist[], int loc_pred[], int loc_n, int n,
              MPI_Comm comm) {

    int i, loc_v, loc_u, glbl_u, new_dist, my_rank, dist_glbl_u;
    int *loc_known;
    int my_min[2];
    int glbl_min[2];

    MPI_Comm_rank(comm, &my_rank);
    loc_known = malloc(loc_n * sizeof(int));

    Dijkstra_Init(loc_mat, loc_pred, loc_dist, loc_known, my_rank, loc_n);

    /* Run loop n - 1 times since we already know the shortest path to global
       vertex 0 from global vertex 0 */
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

        /* For each local vertex (global vertex = loc_v + my_rank * loc_n)
           Update the distances from source vertex (0) to loc_v. If vertex
           is unmarked check if the distance from source to the global u + the
           distance from global u to local v is smaller than the distance
           from the source to local v
         */
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


/*-------------------------------------------------------------------
 * Function:   Find_min_dist
 * Purpose:    find the minimum local distance from the source to the
 *             assigned vertices of the process that calls the method
 *
 *
 * In args:    loc_dist:  array with distances from source 0
 *             loc_known: array with values 1 if the vertex has been visited
 *                        0 if not
 *             loc_n:     local number of vertices
 *
 * Return val: loc_u: the vertex with the smallest value in loc_dist,
 *                    -1 if all vertices are already known
 *
 * Note:       loc_u = -1 is not supposed to be used when this function returns
 *
 */
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

/*-------------------------------------------------------------------
 * Function:  Print_matrix
 * Purpose:   Print the contents of the matrix
 * In args:   mat, rows, cols
 *
 *
 */
void Print_matrix(int mat[], int rows, int cols) {
    int i, j;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++)
            if (mat[i * cols + j] == INFINITY)
                printf("1000000 ");
            else
                printf("%d ", mat[i * cols + j]);
        printf("\n");
    }

    printf("\n");
}

/*-------------------------------------------------------------------
 * Function:    Print_dists
 * Purpose:     Print the length of the shortest path from 0 to each
 *              vertex
 * In args:     n:  the number of vertices
 *              dist:  distances from 0 to each vertex v:  dist[v]
 *                 is the length of the shortest path 0->v
 */
void Print_dists(int global_dist[], int n) {
    int v;

    printf("  v    dist 0->v\n");
    printf("----   ---------\n");

    for (v = 1; v < n; v++) {
        if (global_dist[v] == INFINITY) {
            printf("%3d       %5s\n", v, "inf");
        }
        else
            printf("%3d       %4d\n", v, global_dist[v]);
        }
    printf("\n");
}


/*-------------------------------------------------------------------
 * Function:    Print_paths
 * Purpose:     Print the shortest path from 0 to each vertex
 * In args:     n:  the number of vertices
 *              pred:  list of predecessors:  pred[v] = u if
 *                 u precedes v on the shortest path 0->v
 */
void Print_paths(int global_pred[], int n) {
    int v, w, *path, count, i;

    path =  malloc(n * sizeof(int));

    printf("  v     Path 0->v\n");
    printf("----    ---------\n");
    for (v = 1; v < n; v++) {
        printf("%3d:    ", v);
        count = 0;
        w = v;
        while (w != 0) {
            path[count] = w;
            count++;
            w = global_pred[w];
        }
        printf("0 ");
        for (i = count-1; i >= 0; i--)
            printf("%d ", path[i]);
        printf("\n");
    }

    free(path);
}