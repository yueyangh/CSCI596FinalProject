#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#define INFINITY 1000000
#define MAX_WEIGHT 20

void dijkstra_serial(int *graph, int n, int src, double *time_taken); 
void dijkstra_mpi(int *graph, int n, int my_rank, int p, double *time_taken);

#endif 
