counter=0
NUM_RANKS=8
INPUT_FILE=input20.in
while [ $counter -lt 3 ]; do
  echo "***** Sequential FFT *****"
  mpiexec -n 1 python mpi_fft.py $INPUT_FILE
  echo "***** Parallel FFT using $NUM_RANKS ranks *****"
  mpiexec -n $NUM_RANKS python mpi_fft.py $INPUT_FILE
  let counter+=1
done