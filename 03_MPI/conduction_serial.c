#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char **argv) {
    int N, seed;
    N = atoi(argv[1]);
    seed = atoi(argv[2]);
    srand(seed);
    int temp[N][N];

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            temp[i][j] = random() >> 3; // avoid overflow
        }
    }
    int count = 0, balance = 0;
    int next[N][N];
    while (!balance) {
        count++;
        balance = 1;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                int up = i - 1 < 0 ? 0 : i - 1;
                int down = i + 1 >= N ? i : i + 1;
                int left = j - 1 < 0 ? 0 : j - 1;
                int right = j + 1 >= N ? j : j + 1;
                next[i][j] = (temp[i][j] + temp[up][j] + temp[down][j] +
                        temp[i][left] + temp[i][right]) / 5;
                if (next[i][j] != temp[i][j]) {
                    balance = 0;
                }
            }
        }
        memcpy(temp, next, N * N * sizeof(int));
    }

    printf("Size: %d*%d, Seed: %d, ", N, N, seed);
    printf("Iteration: %d, Temp: %d\n", count, temp[0][0]);
    return 0;
}
