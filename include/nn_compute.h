#ifndef NN_BACKEND_NN_COMPUTE_H
#define NN_BACKEND_NN_COMPUTE_H

#include "nn_backend_macros.h"
#include "nn_types.h"

NN_BACKEND_DLL_EXPORT void
nnRunTwoMatricesAndOutput(NnComputeInfo *info, NnMatrix *matrixA, NnMatrix *matrixB, NnMatrix *matrixC);

NN_BACKEND_DLL_EXPORT void
nnValidCrossCorrelationCpu(NnMatrix *matrix, NnMatrix *kernel, NnMatrix *output);

NN_BACKEND_DLL_EXPORT void
nnMultiply(NnMatrix *matrix, NnMatrix *kernel, NnMatrix *output);

NN_BACKEND_DLL_EXPORT void
nnFullCrossCorrelationCpu(NnMatrix *matrix, NnMatrix *kernel, NnMatrix *output);

NN_BACKEND_DLL_EXPORT void
nnValidCrossCorrelationGpu(NnComputeInfo *info, NnMatrix *matrix, NnMatrix *kernel, NnMatrix *output);

#endif // NN_BACKEND_NN_COMPUTE_H
