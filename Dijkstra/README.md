# Implementation of Dijkstra’s Algorithm using MPI

## 1. Introduction

This project focuses on implementing and analyzing the Dijkstra's algorithm in both serial and parallel formats using MPI (Message Passing Interface). The code and analyses are inspired by the work found in [Lehmannhen's MPI-Dijkstra GitHub repository](https://github.com/Lehmannhen/MPI-Dijkstra/tree/master).

### 1.1 Background

**Dijkstra's Algorithm** is a classic algorithm in computer science that finds the shortest path between a given node (i.e., the "source node") and all other nodes in a graph. It works by systematically selecting the nearest vertex not yet processed, calculating the distance through it to each unvisited neighbor, and updating the neighbor's distance if smaller. ([Source](https://www.freecodecamp.org/news/dijkstras-shortest-path-algorithm-visual-introduction/#:~:text=Dijkstra%27s%20Algorithm%20finds%20the%20shortest,node%20and%20all%20other%20nodes.))

### 1.2 Implementation Overview

1. The graph's adjacency matrix is partitioned by columns among the processes. Each process is assigned $n/p$ number of vertices.
   + $n$: total number of vertices
   + $p$: number of processes
2. Each process computes the local minimum distance from the source vertex to its assigned vertices. This step involves no inter-process communication.
3. The processes collectively determine the global minimum vertex from the source vertex. This vertex is then marked as visited.
4. Each process updates the distance from the source to its assigned vertices. If a shorter path through the global minimum vertex is found, the local distance is updated accordingly.
5. The above steps are repeated (n - 1 times) until the shortest paths from the source to all vertices are computed.

## 2. File
* **dijkstra_serial.c**

  Serial implementation of Dijkstra's algorithm

* **dijkstra_mpi.c**

  Parallel implementation of Dijkstra's algorithm utilizing MPI

* **dijkstra_main.c**

  The entry point for the program. Randomly generated the graph's adjacency matrix and executed both serial and parallel versions of the algorithm
  
* **run_dijkstra_local.sh**

  A shell script used for automating the execution of the program with different numbers of vertices and processes

* **dijkstra.h**

  Includes declarations for functions and constants used across the project's files.

* **dijkstra_local_results.out**

  Output file from running `run_dijkstra_loca.sh`.

## 3. Experiment (Local)

### 3.1 Experimental Setup

* **CPU**: Apple M1 Pro (8 physical cores)
* **Input**: randomly generated weighted directed graphs with 10, 100, 1000, 10000 vertices

### 3.2 Method

1. For each graph size (n), execute serial and parallel versions of Dijkstra's algorithm using 2, 4, 8 processes. Each specific configuration was run 3 times
2. Record the execution time of each run
3. Calculate the average execution time for both the serial and parallel versions 

### 3.3 Experiment Shell Script: `run_dijkstra_loca.sh`

```bash
vertex_counts=(10 100 1000 10000)
process_counts=(2 4 8)
output_file="dijkstra_local_results.out"
> "$output_file"
for n in "${vertex_counts[@]}"; do
    for p in "${process_counts[@]}"; do
        echo "Running for n=$n with $p processes" | tee -a "$output_file"
        for i in {1..3}; do
            echo "Run $i:" | tee -a "$output_file"
            mpiexec -n $p ./dijkstra_program $n 2>&1 | tee -a "$output_file"
        done
        echo "" | tee -a "$output_file"
    done
done
```
+ compile: mpicc -o dijkstra_program dijkstra_main.c dijkstra_serial.c dijkstra_mpi.c
+ run: 
    1. chmod +x run_dijkstra_local.sh
    2. ./run_dijkstra_local.sh

### 3.4 Result

<img src="imgs/local_result_graph.png" style="zoom:80%;" />
+ number of vertices is plotted on a logarithmic scale
+ execution time is presented on a linear scale (seconds)

| Number of Vertices |     Serial     | Parallel (p=2) | Parallel (p=4) | Parallel (p=8) |
|--------------------|----------------|----------------|----------------|----------------|
|                  10|       0.0000049|       0.0000217|        0.000036|     0.000382667|
|--------------------|----------------|----------------|----------------|----------------|
|                 100|       0.0000592|     0.000931667|     0.001346667|     0.003233667|
|--------------------|----------------|----------------|----------------|----------------|
|                1000|       0.0042307|     0.002449667|     0.001907667|     0.005618333|
|--------------------|----------------|----------------|----------------|----------------|
|               10000|       0.4154307|     0.216864667|     0.120905333|        0.229491|

### 3.5 Analysis

The experimental results show that
+ The performance of the parallel Dijkstra's algorithm improves significantly as the number of vertices increases.
+ For smaller graphs (with 10 vertices), the serial implementation tends to perform better due to the overhead of parallelization.
+ There's an optimal number of processes for parallel execution, as seen by the blue (p=2) and green (p=4) lines, which outperform the red line (p=8) for larger graphs, indicating potential overhead from inter-process communication.
