#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INFINITY 1000000
#define MAX_WEIGHT 20

void Generate_random_matrix(int *mat, int n) {
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            mat[i * n + j] = INFINITY;
        }
    }

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (i == j) {
                mat[i * n + j] = 0; 
                mat[j * n + i] = 0;
            } else if (mat[i * n + j] == INFINITY && mat[j * n + i] == INFINITY) {
                int randChoice = rand() % 3;
                if (randChoice == 0) { // 创建从i到j的边 或 从j到i的边 或 无边
                    // 
                    mat[i * n + j] = (rand() % MAX_WEIGHT) + 1;
                } else if (randChoice == 1) {
                    mat[j * n + i] = (rand() % MAX_WEIGHT) + 1;
                }
            }
        }
    }
}

void Print_matrix(int mat[], int rows, int cols) {
    int i, j;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++)
            if (mat[i * cols + j] == INFINITY)
                printf("i ");
            else
                printf("%d ", mat[i * cols + j]);
        printf("\n");
    }

    printf("\n");
}

// 查找距离数组中未处理的最小距离顶点
int minDistance(int dist[], int sptSet[], int n) {
    int min = INFINITY, min_index;
    for (int v = 0; v < n; v++) {
        if (sptSet[v] == 0 && dist[v] <= min) {
            min = dist[v], min_index = v;
        }
    }
    return min_index;
}

// 实现 Dijkstra 算法
void dijkstra(int *graph, int n, int src) {
    int dist[n]; // 存储源到 i 的最短距离
    int sptSet[n]; // 如果顶点 i 在最短路径树或其最短距离已确定，则为 1

    // 初始化所有距离为 INFINITY，sptSet[] 为 0
    for (int i = 0; i < n; i++) {
        dist[i] = INFINITY, sptSet[i] = 0;
    }

    // 源顶点到自身的距离始终为 0
    dist[src] = 0;

    // 查找所有顶点的最短路径
    for (int count = 0; count < n - 1; count++) {
        int u = minDistance(dist, sptSet, n);

        // 标记选中的顶点为已处理
        sptSet[u] = 1;

        // 更新相邻顶点的距离值
        for (int v = 0; v < n; v++) {
            if (!sptSet[v] && graph[u * n + v] && dist[u] != INFINITY && dist[u] + graph[u * n + v] < dist[v]) {
                dist[v] = dist[u] + graph[u * n + v];
            }
        }
    }

    /* 打印结果
    printf("Vertex\tDistance from Source\n");
    for (int i = 0; i < n; i++) {
        printf("%d \t %d\n", i, dist[i]);
    } */
}

int main() {
    int n = 100; // 顶点数
    int *graph = malloc(n * n * sizeof(int));

    // 生成随机矩阵
    srand(time(NULL)); // 初始化随机数生成器
    Generate_random_matrix(graph, n);
    //Print_matrix(graph, n, n);

    // 开始计时
    clock_t start = clock();

    // 执行 Dijkstra 算法
    dijkstra(graph, n, 0);

    // 结束计时
    clock_t end = clock();
    double time_taken = ((double)end - start) / CLOCKS_PER_SEC;
    printf("Dijkstra algorithm took %f seconds to execute \n", time_taken);

    free(graph);
    return 0;
}
