import matplotlib.pyplot as plt
import sys

def main():
    if len(sys.argv) != 3:
        print("Usage: python plot_exec_time.py <exec_time_output_file> <num_ranks>")
        return
    path = sys.argv[1]
    num_ranks = int(sys.argv[2])
    sequential = []
    parallel = []
    with open(path, "r") as f:
        for line in f:
            line = line.split()
            sequential.append(float(line[0]))
            parallel.append(float(line[1]))
    x = list(range(1, len(sequential) + 1))
    plt.plot(x, sequential, label="Sequential FFT", marker="o")
    plt.plot(x, parallel, label="Parallel FFT using {} ranks".format(num_ranks), marker="o")
    xt = list(range(1, len(sequential) + 1))
    plt.xticks(xt)
    plt.xlabel("Run")
    plt.ylabel("Execution Time (s)")
    plt.legend()
    print("Average sequential FFT execution time: {}".format(sum(sequential) / len(sequential)))
    print("Average parallel FFT execution time: {}".format(sum(parallel) / len(parallel)))
    plt.show()


if __name__ == "__main__":
    main()