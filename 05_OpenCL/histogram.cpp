#include <fstream>
#include <iostream>
#include <string>
#include <ios>
#include <vector>

// OpenCL
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>

typedef struct tag_RGB
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t align;
} RGB;

typedef struct tag_Image
{
    bool type;
    uint32_t size;
    uint32_t height;
    uint32_t width;
    RGB *data;
} Image;

Image *readbmp(const char *filename)
{
    std::ifstream bmp(filename, std::ios::binary);
    char header[54];
    bmp.read(header, 54);
    uint32_t size = *(int *)&header[2];
    uint32_t offset = *(int *)&header[10];
    uint32_t w = *(int *)&header[18];
    uint32_t h = *(int *)&header[22];
    uint16_t depth = *(uint16_t *)&header[28];
    if (depth != 24 && depth != 32)
    {
        printf("we don't suppot depth with %d\n", depth);
        exit(0);
    }
    bmp.seekg(offset, bmp.beg);

    Image *ret = new Image();
    ret->type = 1;
    ret->height = h;
    ret->width = w;
    ret->size = w * h;
    ret->data = new RGB[w * h];
    for (int i = 0; i < ret->size; i++)
    {
        bmp.read((char *)&ret->data[i], depth / 8);
    }
    return ret;
}

int writebmp(const char *filename, Image *img)
{

    uint8_t header[54] = {
        0x42,        // identity : B
        0x4d,        // identity : M
        0, 0, 0, 0,  // file size
        0, 0,        // reserved1
        0, 0,        // reserved2
        54, 0, 0, 0, // RGB data offset
        40, 0, 0, 0, // struct BITMAPINFOHEADER size
        0, 0, 0, 0,  // bmp width
        0, 0, 0, 0,  // bmp height
        1, 0,        // planes
        32, 0,       // bit per pixel
        0, 0, 0, 0,  // compression
        0, 0, 0, 0,  // data size
        0, 0, 0, 0,  // h resolution
        0, 0, 0, 0,  // v resolution
        0, 0, 0, 0,  // used colors
        0, 0, 0, 0   // important colors
    };

    // file size
    uint32_t file_size = img->size * 4 + 54;
    header[2] = (unsigned char)(file_size & 0x000000ff);
    header[3] = (file_size >> 8) & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;

    // width
    uint32_t width = img->width;
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8) & 0x000000ff;
    header[20] = (width >> 16) & 0x000000ff;
    header[21] = (width >> 24) & 0x000000ff;

    // height
    uint32_t height = img->height;
    header[22] = height & 0x000000ff;
    header[23] = (height >> 8) & 0x000000ff;
    header[24] = (height >> 16) & 0x000000ff;
    header[25] = (height >> 24) & 0x000000ff;

    std::ofstream fout;
    fout.open(filename, std::ios::binary);
    fout.write((char *)header, 54);
    fout.write((char *)img->data, img->size * 4);
    fout.close();
}

int main(int argc, char *argv[])
{
    
    // declaration
    cl_int errCode;
    cl_platform_id platformId;
    cl_device_id deviceId;
    cl_context context;
    cl_command_queue commandQueue;
    cl_program program;
    cl_kernel kernel;
    cl_mem inputRGB, outputR, outputG, outputB;
    int resultR[256];
    int resultG[256];
    int resultB[256];
    
    // work group size
    size_t localWS[2] = {64, 64};
    size_t globalWS[2];

    // check platform
    clGetPlatformIDs(1, &platformId, NULL);
    
    // check device
    errCode = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_GPU, 1, &deviceId, NULL);
    if (errCode != CL_SUCCESS) {
        printf("clGetDeviceIDs() fails w/ error code = %d\n", errCode);
        return 0;
    }

    // create context
    context = clCreateContext(0, 1, &deviceId, NULL, NULL, &errCode);
    if (!context) {
        printf("clCreateContext() fails w/ error code = %d\n", errCode);
        return 0;
    }
    
    // construct command queue
    commandQueue = clCreateCommandQueue(context, deviceId, 0, &errCode);
    if (!commandQueue) {
        printf("clCreateCommandQueue() fails w/ error code = %d\n", errCode);
        return 0;
    }

    // create program
    int length;
    std::vector<char> data;
    std::ifstream infile("histogram.cl", std::ios_base::in);

    infile.seekg(0, std::ios_base::end);
    length = infile.tellg();
    infile.seekg(0, std::ios_base::beg);
    data = std::vector<char>(length + 1);
    infile.read(&data[0], length);
    data[length] = 0;

    const char *source = &data[0];

    program = clCreateProgramWithSource(context, 1, &source, 0, 0);
    infile.close();
    if (!program) {
        printf("program is NULL\n");
        return 0;
    }
    
    // build program
    errCode = clBuildProgram(program, 0, NULL, NULL, NULL, NULL); 
    if (errCode != CL_SUCCESS) {
        printf("clBuildProgram() fails w/ error code = %d\n", errCode);
        return 0;
    }

    // create kernel
    kernel = clCreateKernel(program, "histogram", &errCode);
    if (!kernel) {
        printf("clCreateKernel() fails w/ error code = %d\n", errCode);
        return 0;
    }

    char *filename;
    if (argc >= 2)
    {
        int many_img = argc - 1;
        for (int i = 0; i < many_img; i++)
        {
            filename = argv[i + 1];
            Image *img = readbmp(filename);

            std::cout << img->width << ":" << img->height << "\n";

            // create buffer
            inputRGB = clCreateBuffer(context, CL_MEM_READ_ONLY, img->size * sizeof(RGB), NULL, NULL);
            outputR = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 256 * sizeof(int), NULL, NULL);
            outputG = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 256 * sizeof(int), NULL, NULL);
            outputB = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 256 * sizeof(int), NULL, NULL);
            if (!inputRGB || !outputR || !outputG || !outputB) {
                printf("clCreateBuffer() fails\n");
                return 0;
            }

            // write buffer (transfer input data to device)
            errCode = clEnqueueWriteBuffer(commandQueue, inputRGB, CL_TRUE, 0, img->size * sizeof(RGB), img->data, 0, NULL, NULL);
            
            // arguments
            errCode = clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputRGB);
            errCode = clSetKernelArg(kernel, 1, sizeof(int), &img->size);
            errCode = clSetKernelArg(kernel, 2, sizeof(cl_mem), &outputR);
            errCode = clSetKernelArg(kernel, 3, sizeof(cl_mem), &outputG);
            errCode = clSetKernelArg(kernel, 4, sizeof(cl_mem), &outputB);
            if (errCode != CL_SUCCESS) {
                printf("ckSetKernekArg() fails w/ error code = %d\n", errCode);
                return 0;
            }
            
            // detemine global work group size
            if (img->size % localWS[0] != 0) {
                int remainder = img->size % localWS[0];
                globalWS[0] = img->size - remainder + localWS[0];
            } else {
                globalWS[0] = img->size;
            }
            if (img->size % localWS[1] != 0) {
                int remainder = img->size % localWS[1];
                globalWS[1] = img->size - remainder + localWS[1];
            } else {
                globalWS[1] = img->size;
            }
            
            // execution
            errCode = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, globalWS, localWS, 0, NULL, NULL);
            if (errCode != CL_SUCCESS) {
                printf("clEnqueueNDRangeKernel() fails w/ error code = %d\n", errCode);
                return 0;
            }
            
            clFinish(commandQueue);

            // read results
            errCode = clEnqueueReadBuffer(commandQueue, outputR, CL_TRUE, 0, 256 * sizeof(int), resultR, 0, NULL, NULL);
            errCode = clEnqueueReadBuffer(commandQueue, outputG, CL_TRUE, 0, 256 * sizeof(int), resultG, 0, NULL, NULL);
            errCode = clEnqueueReadBuffer(commandQueue, outputB, CL_TRUE, 0, 256 * sizeof(int), resultB, 0, NULL, NULL);
            if (errCode != CL_SUCCESS) {
                printf("clEnqueueReadBuffer() fails w/ error code = %d\n", errCode);
                return 0;
            }

            int max = 0;
            for(int i  = 0; i < 256; i++){
                max = resultR[i] > max ? resultR[i] : max;
                max = resultG[i] > max ? resultG[i] : max;
                max = resultB[i] > max ? resultB[i] : max;
            }

            // 256 by 256 histogram picture
            Image *ret = new Image();
            ret->type = 1;
            ret->height = 256;
            ret->width = 256;
            ret->size = 256 * 256;
            ret->data = new RGB[256 * 256]{}; // new and initialize to 0

            // define RBG value (0 or 255) for each pixel
            for(int i = 0; i < ret->height; i++) {
                for(int j = 0; j < ret->width; j++) {
                    if(resultR[j] * 256 / max > i)
                        ret->data[256 * i + j].R = 255;
                    if(resultG[j] * 256 / max > i)
                        ret->data[256 * i + j].G = 255;
                    if(resultB[j] * 256 / max > i)
                        ret->data[256 * i + j].B = 255;
                }
            }

            std::string newfile = "hist_" + std::string(filename); 
            writebmp(newfile.c_str(), ret);
	    delete [] ret;
        }
    } else {
        printf("Usage: ./hist <img.bmp> [img2.bmp ...]\n");
    }

    // release
    clReleaseMemObject(inputRGB);
    clReleaseMemObject(outputR);
    clReleaseMemObject(outputG);
    clReleaseMemObject(outputB);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commandQueue);
    clReleaseContext(context);

    return 0;
}
