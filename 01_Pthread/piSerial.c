#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// global variables
unsigned long long dartInCircle = 0;
unsigned long long toss = 10000000;

// toss dart function
void tossDart() {
    // local variables
    long double max = 1, min = -1; // boundary of random number
    long double x, y, distance;

    // get random seed
    srand(time(NULL));

    // toss the dart
    for (int i = 0; i < toss; i++) {
        x = (max - min) * ((long double) rand()) / (RAND_MAX + 1.0) + min;
        y = (max - min) * ((long double) rand()) / (RAND_MAX + 1.0) + min;
        if (x * x + y * y <= 1) {
            dartInCircle++;
        }
    }
    return;
}

int main(int argc, char *argv[]) {
    // parse arguments
    if (argc != 3) {
        printf("Incorrect # of arguments!\n");
        return 0;
    }

    int core = atoi(argv[1]);
    toss = atoll(argv[2]);

    // toss darts
    tossDart();

    // calculate pi
    long double pi_estimate = 4 * ((long double) dartInCircle) / ((long double) toss);
    printf("\n----- Experimental Results -----\n");
    printf("# of darts in circle = %llu\n", dartInCircle);
    printf("# of tosses = %llu\n", toss);
    printf("estimated pi = %Lf\n\n", pi_estimate);
    return 0;
}
