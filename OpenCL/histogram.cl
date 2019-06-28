// OpenCL kernel function

typedef struct tag_RGB
{
    uchar R; // uchar = unsigned char: represents [0, 255]
    uchar G;
    uchar B;
    uchar align;
} RGB;

// kernel function
__kernel void histogram(__global RGB *data, int inputSize, __global int *outputR, __global int *outputG, __global int *outputB) {
    // variables
    int threadId = get_global_id(0);
    
    // accumulation
    if (threadId < inputSize) {
        // initialization
        RGB pixel = data[threadId];
        
        // histogram
        atomic_inc(&outputR[pixel.R]);
        atomic_inc(&outputG[pixel.G]);
        atomic_inc(&outputB[pixel.B]);
    }
}
