#ifndef THREADDATA_H
#define THREADDATA_H

#include "byte_fifo.h"

struct thread_data
{
    CByteFifo* fifo;

    const uint8* testData;
    uint32 testDataSize;

    bool   errorFlag;
    uint32 countProcessed;

    uint32 threadId;
};

#endif // THREADDATA_H
