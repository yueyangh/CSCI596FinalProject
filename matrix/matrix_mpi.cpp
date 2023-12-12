#include<iostream>
#include<mpi.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>
#include<limits.h>
#include<float.h>

using namespace std;

const float EPS = 1e-6;
const int MAX_SPEEDUP = INT_MAX;

void initMatrix(float *A, int rows, int cols)
{
    for(int i = 0; i < rows*cols; i++){
        A[i] = 0;
    }
}

void initMatrixRandom(float *A, int rows, int cols)
{
    srand((unsigned)time(NULL));
    for(int i = 0; i < rows*cols; i++){
        A[i] = (float)rand() / RAND_MAX;
    }
}

void matrixMultiply(float *A, float *B, float *result, int m, int p, int n)
{
    for(int i=0; i<m; i++){
        for(int j=0; j<n; j++){
            float temp = 0;
            for(int k=0; k<p; k++){
                temp += A[i*p+k] * B[k*n + j];
            }
            result[i*n+j] = temp;
        }
    }
}

float max(float *vec, int length)
{
    float max = FLT_MIN;
    for(int i=0; i<length; i++){
        if(vec[i] > max){
            max = vec[i];
        }
    }
    return max;
}

int main(int argc, char** argv)
{
    if(argc != 4){
        cout << "Error with 3 arguments m, p, n." << endl;
    }
    int m = atoi(argv[1]);
    int p = atoi(argv[2]);
    int n = atoi(argv[3]);
    
    float *A, *B, *C, *C_true;
    float *bA, *bC;
    float elapseTime, elapseTimeSerial, *elapseTimeRecv;
 
    int myrank, numprocs;

    MPI_Status status;
  
    MPI_Init(&argc, &argv); 
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs); 
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    
    if(numprocs < 2){
        cout << "Error: processors should more than 2." << endl;
    }
    
    int bm = m / numprocs;

    bA = new float[bm * p];
    B  = new float[p * n];
    bC = new float[bm * n];
    initMatrix(bA, bm, p);
    initMatrix(B, p, n);
    initMatrix(bC, bm, n);
    
    if(myrank == 0){
	elapseTimeRecv = new float[numprocs];
        A = new float[m * p];
        C = new float[m * n];
        C_true = new float[m * n];
        
        initMatrixRandom(A, m, p);
        initMatrixRandom(B, p, n);
        initMatrix(C, m, n);
        initMatrix(C_true, m, n);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    clock_t start_time = clock();    

    clock_t t_temp_start = clock();
    MPI_Scatter(A, bm * p, MPI_FLOAT, bA, bm *p, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, p * n, MPI_FLOAT, 0, MPI_COMM_WORLD);
    clock_t t_temp_end = clock();

    matrixMultiply(bA, B, bC, bm, p, n);
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    MPI_Gather(bC, bm * n, MPI_FLOAT, C, bm * n, MPI_FLOAT, 0, MPI_COMM_WORLD);
  
    int remainRowsStartId = bm * numprocs;
    if(myrank == 0 && remainRowsStartId < m){
        int remainRows = m - remainRowsStartId;
        matrixMultiply(A + remainRowsStartId * p, B, C + remainRowsStartId * n, remainRows, p, n);
    }
    
    clock_t end_time = clock();	

    elapseTime = (float)(end_time-start_time) / CLOCKS_PER_SEC;
        
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Gather(&elapseTime, 1, MPI_FLOAT, elapseTimeRecv, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

    if(myrank == 0){
        elapseTime = max(elapseTimeRecv, numprocs);
		cout << "Cost " << elapseTime << " seconds in parallel." << endl;

        start_time = clock();
        matrixMultiply(A, B, C_true, m, p, n);
        end_time = clock();

		elapseTimeSerial = float(end_time-start_time) / CLOCKS_PER_SEC;
		cout << "Cost " << elapseTimeSerial << " seconds in serial." << endl;
    }
     
    delete[] bA;
    delete[] B;
    delete[] bC;
    
    if(myrank == 0){
		delete[] elapseTimeRecv;
        delete[] A;
        delete[] C;
        delete[] C_true;
    }
    
    MPI_Finalize();

    return 0;
}   






