#include <stdio.h>
#include <stdlib.h>

#include "nn_filesystem.h"
#include "nn_utils.h"

uint8_t *nnReadBinaryFile(const char *path, uint64_t *read_count) {
    FILE *file;
    uint8_t *data;

    file = fopen(path, "rb");
    if (file == NULL) {
        NN_PRINTF("[WARNING]: Failed to open file %s\n", path);
        *read_count = 0;
        return NULL;
    }

    // get file size
    fseek(file, 0, SEEK_END);
    *read_count = ftell(file);
    rewind(file);

    // allocate memory and read bytes from file and close file.
    data = (uint8_t *) malloc(*read_count);
    fread(data, *read_count, 1, file);
    fclose(file);

    return data;
}

NnStatus nnWriteBinaryFile(const char *path, const uint8_t *data, const size_t write_count) {
    FILE *file;

    file = fopen(path, "ab+");
    if (file == NULL) {
        NN_PRINTF("[WARNING]: Failed to open file %s\n", path);
        return NN_FAILURE;
    }

    fwrite(data, write_count, 1, file);
    fclose(file);

    return NN_SUCCESS;
}
