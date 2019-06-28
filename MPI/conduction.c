#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define TAG0 100
#define TAG1 101
#define TAG2 102
#define TAG3 103
#define TAG4 104
#define TAG5 105
#define TAG6 106

int main(int argc, char **argv) {

    // initialization
    int N, seed;
    N = atoi(argv[1]);
    seed = atoi(argv[2]);
    srand(seed);
    int count = 0, balance = 0, _balance = 0;
    int temp[N][N], next[N][N];
    int container[2 * N + 1];

    // MPI status
    MPI_Status status;
    MPI_Request request;

    // MPI initialization
    int size;
    int rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // load distribution
    int portion;
    if (N % size == 0) {
        portion = N / size;
    } else {
        portion = N / size + 1;
    }
    int start = portion * rank;
    int end = portion * (rank + 1);

    // initialize temperature matrix
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            temp[i][j] = random() >> 3; // avoid overflow
        }
    }

    // conduction
    while (!balance) {
        count++;
        balance = 1;

        for (int i = start; i < end && i < N; i++) {
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

        // master thread
        if (rank == 0) {

            // reveive balance & boundary
            for (int i = 1; i < size; i++) {
                if (i < size - 1) {
                    MPI_Recv(container, 2 * N + 1, MPI_INT, i, TAG0 * i, MPI_COMM_WORLD, &status);
                    memcpy(&next[portion * i][0], &container[0], N * sizeof(int));
                    memcpy(&next[portion * (i + 1) - 1][0], &container[N], N * sizeof(int));
                    balance = balance * container[2 * N];
                } else {
                    MPI_Recv(container, N + 1, MPI_INT, i, TAG0 * i, MPI_COMM_WORLD, &status);
                    memcpy(&next[portion * i][0], &container[0], N * sizeof(int));
                    balance = balance * container[N];
                }
            }

            // send balance & boundary
            for (int i = 1; i < size; i++) {
                if (i < size - 1) {
                    container[2 * N] = balance;
                    memcpy(&container[0], &next[portion * i - 1], N * sizeof(int));
                    memcpy(&container[N], &next[portion * (i + 1)], N * sizeof(int));
                    MPI_Send(container, 2 * N + 1, MPI_INT, i, TAG4 * i, MPI_COMM_WORLD);
                } else {
                    container[N] = balance;
                    memcpy(&container[0], &next[portion * i - 1], N * sizeof(int));
                    MPI_Send(container, N + 1, MPI_INT, i, TAG4 * i, MPI_COMM_WORLD);
                }
            }

            // copy for next iteration
            memcpy(&temp[0][0], &next[0][0], (portion + 1) * N * sizeof(int));
        }

        // slave thread(s)
        if (rank > 0) {

            // send boundary
            if (rank < size - 1) {
                memcpy(&container[0], &next[start][0], N * sizeof(int));
                memcpy(&container[N], &next[end - 1][0], N * sizeof(int));
                container[2 * N] = balance;
                MPI_Send(container, 2 * N + 1, MPI_INT, 0, TAG0 * rank, MPI_COMM_WORLD);
            } else {
                memcpy(&container[0], &next[start][0], N * sizeof(int));
                container[N] = balance;
                MPI_Send(container, N + 1, MPI_INT, 0, TAG0 * rank, MPI_COMM_WORLD);
            }

            // receive boundary
            if (rank < size - 1) {
                MPI_Recv(container, 2 * N + 1, MPI_INT, 0, TAG4 * rank, MPI_COMM_WORLD, &status);
                memcpy(&next[start - 1][0], &container[0], N * sizeof(int));
                memcpy(&next[end][0], &container[N], N * sizeof(int));
                balance = container[2 * N];
            } else {
                MPI_Recv(container, N + 1, MPI_INT, 0, TAG4 * rank, MPI_COMM_WORLD, &status);
                memcpy(&next[start - 1][0], &container[0], N * sizeof(int));
                balance = container[N];
            }

            // copy for next iteration
            if (rank < size - 1) {
              memcpy(&temp[start - 1][0], &next[start - 1][0], (portion + 2) * N * sizeof(int));
            } else {
              memcpy(&temp[start - 1][0], &next[start - 1][0], (portion + 1) * N * sizeof(int));
            }
        }
    }

    // final report
    if (rank == 0) {
        printf("Size: %d*%d, Seed: %d, ", N, N, seed);
        printf("Iteration: %d, Temp: %d\n", count, temp[0][0]);
    }

    MPI_Finalize();
    return 0;
}
