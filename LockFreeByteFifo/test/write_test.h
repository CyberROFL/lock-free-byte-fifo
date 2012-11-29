#ifndef WRITETEST_H
#define WRITETEST_H

#include "byte_fifo.h"

void write_test(int nThreads,
                uint32 fifoSize,
                const uint8* testData,
                uint32 testDataSize);

#endif // WRITETEST_H
