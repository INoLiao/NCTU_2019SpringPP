#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// global variables
unsigned long long dartInCircle = 0;
unsigned long long toss;
unsigned long long tossPerThread;
pthread_mutex_t mutex;

// toss dart function
void* tossDart() {
    // local variables
    unsigned long long count = 0;
    unsigned int seed = time(NULL);

    // toss the dart
    for (unsigned long long i = 0; i < tossPerThread; i++) {
        double x = 2.0 * (double) rand_r(&seed) / (RAND_MAX + 1.0) - 1.0;
		double y = 2.0 * (double) rand_r(&seed) / (RAND_MAX + 1.0) - 1.0;

        if (x * x + y * y <= 1) {
            count++;
        }
    }

    // critical section (mutex)
    pthread_mutex_lock(&mutex);
    dartInCircle += count;
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

int main(int argc, char *argv[]) {
    // parse arguments
    if (argc != 3) {
        printf("Incorrect # of arguments!\n");
        return 0;
    }

    int numThread = atoi(argv[1]);
    toss = atoll(argv[2]);
    tossPerThread = toss / numThread;

    // pthread initialization
    pthread_t *threads = malloc(numThread * sizeof(pthread_t));
    pthread_mutex_init(&mutex, NULL);
    
    // create threads
    for (unsigned long long i = 0; i < numThread; i++) {
        pthread_create(&threads[i], NULL, tossDart, NULL);
    }

    // join threads
    for (unsigned long long i = 0; i < numThread; i++) {
        pthread_join(threads[i], NULL);
    }

    // release threads
    free(threads);
    pthread_mutex_destroy(&mutex);

    // calculate pi
    long double pi_estimate = 4 * ((long double) dartInCircle) / ((long double) toss);
    printf("pi_estimate = %Lf\n\n", pi_estimate);

    return 0;
}
