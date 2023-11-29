# Description: This program implements the parallel FFT algorithm using MPI.
# Author: Tan Huang (tanhuang@usc.edu)
# Date: 2023-11-28

import numpy as np
import mpi4py.MPI as MPI
import sys
import os

# from scipy.fft import fft as scipy_fft

def bin_rev(x, n):
    """Reverse the binary representation of x, assuming n bits."""
    rev_x = 0
    for i in range(n):
        rev_x = (rev_x << 1) | (x & 1)
        x >>= 1
    return rev_x

def main():
    if len(sys.argv) != 2:
        print("Usage: python mpi_fft.py <input_file>")
        return
    
    if not os.path.isfile(sys.argv[1]):
        print("file {} does not exist".format(sys.argv[1]))
        return

    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()
    size = comm.Get_size()
    if (size & (size - 1)) != 0:
        print("The number of ranks must be a power of 2")
        return
    x = None
    x_re = None
    num_points = None
    #correct_x = None
    start = None
    end = None

    #print(MPI.Get_processor_name())

    if rank == 0:
        with open(sys.argv[1], "r") as f:
            x = np.array([complex(num) for num in f.read().split()])
        # correct_x = scipy_fft(x)
        # print("Expected: {}".format(correct_x))
        # print("rank {} read input".format(rank))
        num_points = len(x)
        x_re = x.reshape((1, num_points))
        if num_points <= 0:
            print("The number of input points must be positive")
            num_points = -1
        elif (num_points & (num_points - 1)) != 0:
            print("The number of input points must be a power of 2")
            num_points = -1
        elif size >= num_points: # if the number of processes is equal to or greater than the number of points
            size = num_points // 2  # the number of processes is set to the number of points / 2
        
        start = MPI.Wtime()
        # Rearrange the input into reverse binary order.
        for i in range(num_points):
            j = bin_rev(i, num_points.bit_length() - 1)
            if i < j:
                x[i], x[j] = x[j], x[i]

    num_points = comm.bcast(num_points, root=0) # broadcast the number of points to all processes
    #print("rank {} broadcast".format(rank))
    if num_points == 1:
        if rank == 0:
            print("Output: {}".format(x))
        return
    elif num_points == -1:
        return
    
    sub_num_points = num_points // size
    sub_x = np.empty(sub_num_points, dtype=complex)
    comm.Scatter(x_re, sub_x, root=0) # scatter the input to all processes
    #print("rank {} scatter".format(rank))

    # Perform butterfly operations without communication
    stage_size = sub_num_points.bit_length() - 1
    for stage in range(1, stage_size + 1):
        group_size = 1 << stage
        omega = np.exp(-2j * np.pi / group_size)
        z = complex(1)
        for round in range(group_size // 2): # each round has the same omega value
            for idx in range(round, sub_num_points, group_size):
                t = z * sub_x[idx + group_size // 2]
                u = sub_x[idx]
                sub_x[idx] = u + t
                sub_x[idx + group_size // 2] = u - t
            z *= omega

    # Perform butterfly operations with communication
    if size > 1:
        sub_t = np.empty(sub_num_points, dtype=complex)
        sub_u = np.empty(sub_num_points, dtype=complex)
        for s in range(1, size.bit_length()): # number of the remaining stages is size.bit_length() - 1 (log[size])
            group_size = 1 << (s + sub_num_points.bit_length() - 1) # group size is 2^(s + log[sub_num_points])
            omega = np.exp(-2j * np.pi / group_size)
            if (rank // (1 << (s - 1))) % 2 == 1:
                pos = (rank * sub_num_points) % (group_size // 2)
                z = np.exp(-2j * np.pi * pos / group_size)
                for j in range(sub_num_points):
                    sub_t[j] = z * sub_x[j]
                    z *= omega
            shift = 1 << (s - 1)
            partner = rank ^ shift
            if (rank // (1 << (s - 1))) % 2 == 1:
                req_s = comm.Isend(sub_t, dest=partner, tag=s)
                req_r = comm.Irecv(sub_u, source=partner, tag=s)
                req_r.Wait()
                #print("! rank {} recv data from rank {}, tag = {}".format(rank, partner, s))
                for j in range(sub_num_points):
                    sub_x[j] = sub_u[j] - sub_t[j]
                req_s.Wait()
                #print("! rank {} send data to rank {}, tag = {}".format(rank, partner, s))
            else:
                req_s = comm.Isend(sub_x, dest=partner, tag=s)
                req_r = comm.Irecv(sub_t, source=partner, tag=s)
                req_r.Wait()
                #print("rank {} recv data from rank {}, tag = {}".format(rank, partner, s))
                req_s.Wait()
                #print("rank {} send data to rank {}, , tag = {}".format(rank, partner, s))
                for j in range(sub_num_points):
                    sub_x[j] += sub_t[j]
        # Gather the results
        comm.Gather(sub_x, x, root=0)
        #print("rank {} gather".format(rank))
    else:
        x = sub_x
    end = MPI.Wtime()
    if rank == 0:
        #print("Output: {}".format(x))
        #print(np.allclose(x, correct_x))
        print("Execution time (s): {}".format(end - start))
        # name = sys.argv[1].split(".")[0]
        # name = name.split("input")[1]
        # name = "output{}.out".format(name)
        # with open(name, "w") as f:
        #     for num in x:
        #         f.write("{}\n".format(num))
    
if __name__ == "__main__":
    main()