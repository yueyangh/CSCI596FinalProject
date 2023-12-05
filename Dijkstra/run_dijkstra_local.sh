#!/bin/bash

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
