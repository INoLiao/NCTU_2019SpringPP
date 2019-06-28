/**********************************************************************
 * DESCRIPTION:
 *   Parallel Concurrent Wave Equation w/ CUDA acceleration
 *   This program implements the concurrent wave equation
 *********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAXPOINTS 1000000
#define MAXSTEPS 1000000
#define MINPOINTS 20
#define PI 3.14159265

// CUDA
#define BLOCK_SIZE 1024

// function declaration
void check_param(void);
void update (void);
void printfinal (void);

// global variable
int nsteps,                 	 /* number of time steps */
    tpoints, 	     		 /* total points along string */
    rcode;                  	 /* generic return code */
float values[MAXPOINTS + 2]; 	 /* values at time t */

// CUDA
float  *Vd;

/**********************************************************************
 *	Checks input values from parameters
 *********************************************************************/
void check_param(void)
{
    char tchar[20];

    // check number of points, number of iterations
    while ((tpoints < MINPOINTS) || (tpoints > MAXPOINTS)) {
        printf("Enter number of points along vibrating string [%d-%d]: "
                ,MINPOINTS, MAXPOINTS);
        scanf("%s", tchar);
        tpoints = atoi(tchar);
        if ((tpoints < MINPOINTS) || (tpoints > MAXPOINTS))
            printf("Invalid. Please enter value between %d and %d\n",
                    MINPOINTS, MAXPOINTS);
    }

    while ((nsteps < 1) || (nsteps > MAXSTEPS)) {
        printf("Enter number of time steps [1-%d]: ", MAXSTEPS);
        scanf("%s", tchar);
        nsteps = atoi(tchar);
        if ((nsteps < 1) || (nsteps > MAXSTEPS))
            printf("Invalid. Please enter value between 1 and %d\n", MAXSTEPS);
    }

    printf("Using points = %d, steps = %d\n", tpoints, nsteps);

}

/**********************************************************************
 *    CUDA acceleration code block
 *    - Initialize points on line
 *    - Calculate new values using wave equation and update accordingly
 *********************************************************************/
__global__ void runOnGPU(float *Vd, int tpoints, int nsteps)
{
    // variable declaration
    int i, k;
    float x, fac, tmp;
    float dtime, c, dx, tau, sqtau;
    float value, newVal, oldVal;

    // init_line()
    fac = 2.0 * PI;
    k = 1 + blockIdx.x * BLOCK_SIZE + threadIdx.x;
    tmp = tpoints - 1;
    x = (k - 1) / tmp;
    value = sin(fac * x);
    oldVal = value;

    // do_math() + update()
    dtime = 0.3;
    c = 1.0;
    dx = 1.0;
    tau = (c * dtime / dx);
    sqtau = tau * tau;

    if(k <= tpoints) {

        // propagate for nstpes iterations
        for (i = 1; i <= nsteps; i++) {

            // check boundary
            if ((k == 1) || (k == tpoints))
                newVal = 0.0;
            else
                newVal = (2.0 * value) - oldVal + (sqtau * (-2.0) * value);

            // update oldVal and value
            oldVal = value;
            value = newVal;
        }
        
        // update final value to Vd
        Vd[k] = value;
    }
}

/**********************************************************************
 *     Print final results
 *********************************************************************/
void printfinal()
{
    int i;

    for (i = 1; i <= tpoints; i++) {
        printf("%6.4f ", values[i]);
        if (i % 10 == 0)
            printf("\n");
    }
}

/**********************************************************************
 *	Main program
 *********************************************************************/
int main(int argc, char *argv[])
{
    // variable declaration
    int size;
    int blockNum;

    // parse arguments
    sscanf(argv[1], "%d", &tpoints);
    sscanf(argv[2], "%d", &nsteps);
    check_param();

    printf("Initializing points on the line...\n");
    printf("Updating all points for all time steps...\n");

    // cuda memory allocation
    size = (tpoints + 1) * sizeof(float);
    cudaMalloc((void**) &Vd, size);

    // CUDA
    if (tpoints % BLOCK_SIZE == 0) {
        blockNum = tpoints / BLOCK_SIZE;
    } else {
        blockNum = tpoints / BLOCK_SIZE + 1;
    }
    runOnGPU<<<blockNum, BLOCK_SIZE>>>(Vd, tpoints, nsteps);
    cudaMemcpy(values, Vd, size, cudaMemcpyDeviceToHost);
    cudaFree(Vd);

    printf("Printing final results...\n");
    printfinal();
    printf("\nDone.\n\n");

    return 0;
}
