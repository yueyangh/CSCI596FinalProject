import numpy as np
import sys

def main():
    if len(sys.argv) != 2:
        print("Usage: python input_generator.py <num>")
        print("Example: 'python input_generator.py 10' generates 2^10 = 1024 random numbers in input10.in")
        return
    power_of_2 = int(sys.argv[1])
    file_name = "input{}.in".format(power_of_2)
    with open(file_name, "w") as f:
        for i in range(2 ** power_of_2):
            f.write("{}\n".format(np.random.rand() * 10))

if __name__ == "__main__":
    main()