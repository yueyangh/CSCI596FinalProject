m=1000
p=2000
n=3000
np=4

echo -e "\n====== Run test ======\n"
echo -e "\nTest matrix multiply with MPI"
echo "np=$np, m=$m, p=$p, n=$n"

mpirun -np $np matrix_mpi $m $p $n

echo -e "\n===== Test finish =====\n"
