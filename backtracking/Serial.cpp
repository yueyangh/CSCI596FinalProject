
#include <iostream>
#include <ctime>
using namespace std;

bool resultfind = false;

bool canput(int* x, int a){
    for(int i = 0; i < a; i++){
        if(x[i] == x[a] || x[i] + i == x[a] + a || x[i] - i == x[a] - a){
            return false;
        }
    }
    return true;
}

/*backtracking function, n means how many rows and columns there are*/
bool solve(int* x, int n){
    int k = 0;
    x[0] = -1;
    while(k >= 0 && !resultfind){
        x[k] += 1;
        while((x[k] < n) && !(canput(x, k))){
            x[k] += 1;
        }
        if(x[k] < n){
            if(k == n - 1){
                resultfind = true;
                return true;
            }else{
                k++;
                x[k] = -1;
            }
        }else{
            k--;
        }   
    }  
    return false; 
}

int main() {
    int n;
    cout << "Enter the value of n: ";
    cin >> n;

    int* x = new int[n];

    clock_t start_time = clock();
    solve(x, n);
    clock_t end_time = clock();

    if (resultfind) {
        cout << "Solution found!" << endl;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (x[i] == j) {
                    cout << "Q ";
                } else {
                    cout << ". ";
                }
            }
            cout << endl;
        }
    } else {
        cout << "No solution exists." << endl;
    }

    // print the output time 
    double elapsed_time = double(end_time - start_time) / CLOCKS_PER_SEC;
    cout << "Time elapsed: " << elapsed_time << " seconds" << endl;

    // free memory
    delete[] x;

    return 0;
}