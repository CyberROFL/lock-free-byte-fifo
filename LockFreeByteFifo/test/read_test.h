#ifndef READTEST_H
#define READTEST_H

#include "byte_fifo.h"

void read_test(int nThreads,
               uint32 fifoSize,
               const uint8* testData,
               uint32 testDataSize);

#endif // READTEST_H
