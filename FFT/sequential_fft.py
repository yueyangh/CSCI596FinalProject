import numpy as np
from scipy.fftpack import fft as scipy_fft

def bin_rev(x, n):
    """Reverse the binary representation of x, assuming n bits."""
    rev_x = 0
    for i in range(n):
        rev_x = (rev_x << 1) | (x & 1)
        x >>= 1
    return rev_x

def fft(x):
    """Compute the FFT of x, assuming len(x) is a power of 2."""
    num_points = len(x)
    assert num_points > 0 and (num_points & (num_points - 1)) == 0, "len(x) must be a power of 2"

    x = x.astype(complex)

    # Rearrange the input into reverse binary order.
    for i in range(num_points):
        j = bin_rev(i, num_points.bit_length() - 1)
        if i < j:
            x[i], x[j] = x[j], x[i]

    # Perform the FFT.
    stage_size = num_points.bit_length() - 1
    for stage in range(1, stage_size + 1):
        group_size = 1 << stage
        omega = np.exp(-2j * np.pi / group_size)
        z = complex(1)
        for round in range(group_size // 2): # each round has the same omega value
            for idx in range(round, num_points, group_size):
                t = z * x[idx + group_size // 2]
                u = x[idx]
                x[idx] = u + t
                x[idx + group_size // 2] = u - t
            z *= omega
    return x

def main():
    """Test the FFT implementation."""
    x = np.random.rand(8)
    print(fft(x))
    print(scipy_fft(x))
    assert np.allclose(fft(x), scipy_fft(x))


if __name__ == "__main__":
    main()