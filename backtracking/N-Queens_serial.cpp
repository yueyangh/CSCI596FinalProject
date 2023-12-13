#include <iostream>
#include <ctime>
using namespace std;

bool resultfind = false;

/* if a queen can be set in line a */
bool canput(int* x, int a) {
    for (int i = 0; i < a; i++) {
        if (x[i] == x[a] || x[i] + i == x[a] + a || x[i] - i == x[a] - a) {
            return false;
        }
    }
    return true;
}


bool solve(int* x, int n) {
    int k = 1;
    x[1] = -1;
    while (k > 0 && !resultfind) {
        x[k] += 1;
        while ((x[k] < n) && !(canput(x, k))) {
            x[k] += 1;
        }
        if (x[k] < n) {
            if (k == n - 1) {
                resultfind = true;
                return true;
            } else {
                k++;
                x[k] = -1;
            }
        } else {
            k--;
        }
    }
    return false;
}

void nQueens(int* x, int n) {
    resultfind = false;
    for (int i = 0; i < n; i++) {
        int* y = new int[n];
        y[0] = i;
        if (solve(y, n)) {
            for (int j = 0; j < n; j++) {
                x[j] = y[j];
            }
        }
        delete[] y;
    }
}

int main() {
    for (int n = 19; n < 33; n++) {
        int* x = new int[n];
        clock_t t = clock();
        nQueens(x, n);
        t = clock() - t;

        // Check for errors in the solution
        for (int i = 0; i < n; i++) {
            if (x[i] < 0 || x[i] >= n) {
                cout << "error!" << endl;
                break;
            }
        }
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                if (x[i] == x[j] || x[i] + i == x[j] + j || x[i] - i == x[j] - j) {
                    cout << "error!" << endl;
                    break;
                }
            }
        }

        // Print the output in the desired format
        cout << "n = " << n << ", Cost " << (double)t / CLOCKS_PER_SEC << " seconds in serial." << endl;

        delete[] x;

        cout << "---------------------" << endl;
    }
    return 0;
}