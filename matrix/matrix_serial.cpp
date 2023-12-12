#include<iostream>
#include<math.h>
#include<stdlib.h>
#include<time.h>

using namespace std;

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

int main(int argc, char** argv)
{
	cout << "Please input 3 arguments m,p,n." << endl;
    if(argc < 4){
        cout << "Error with 3 arguments m, p, n." << endl;
    }
    
    int m = atoi(argv[1]);
    int p = atoi(argv[2]);
    int n = atoi(argv[3]);
	int repeats = 1;
	if(argc == 5){
		repeats = atoi(argv[4]);
	}
    
    float *A, *B, *C;
    
    A = new float[m * p];
    B = new float[p * n];
    C = new float[m * n];
        
	double avgtime = 0.0;
	for(int r = 0; r < repeats; r++){ 	
	    initMatrixRandom(A, m, p);     
	    initMatrixRandom(B, p, n);
	    initMatrix(C, m, n);
        
	    clock_t start_time = clock();    

	    matrixMultiply(A, B, C, m, p, n);
    
	    clock_t end_time = clock();	
		avgtime += (end_time - start_time) / CLOCKS_PER_SEC;
	}
	avgtime = avgtime / repeats;

	cout << "m = " << m << "cost" << avgtime << " seconds." << endl;
     
    delete[] A;
    delete[] B;
    delete[] C;
    	
    return 0;
}

