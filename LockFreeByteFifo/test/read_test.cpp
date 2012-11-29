#include "stdafx.h"

#include "read_test.h"
#include "thread_data.h"
#include "test_result.h"

#include <iostream>
#include <cassert>

static DWORD WINAPI read_thread(LPVOID lParam);

void read_test(int nThreads,
               uint32 fifoSize,
               const uint8* testData,
               uint32 testDataSize)
{
    /*
        We create a single fifo with 'fifoSize' elements.
        We populate the fifo, where the user data is a 'testData'.

        We create 'nThreads' threads where each thread busy-works,
        reading until the fifo is empty.

        Each thread keep track of the number of reads it manages
        and that each user data it reads is equal to the 'testData'.
    */

    std::cout << "Read test:\t";

    assert (!(fifoSize % testDataSize));

    CByteFifo* fifo = new CByteFifo(fifoSize);

    assert (fifo->GetDataSize() == 0);

    // Populate
    for (uint32 i = 0; i < fifoSize / testDataSize; ++i)
    {
        uint32 actualSize;
        uint8* wblk;

        wblk = fifo->WriteAlloc(testDataSize, actualSize);

        assert (!!wblk);
        assert (actualSize == testDataSize);

        memcpy_s(wblk, actualSize, testData, testDataSize);

        fifo->WriteCommit(wblk, actualSize);
    }

    assert (fifo->GetDataSize() == fifoSize);

    // Run readers
    thread_data* data = new thread_data[nThreads];
    HANDLE* threads   = new HANDLE[nThreads];

    for (int i = 0; i < nThreads; ++i)
    {
        data[i].fifo           = fifo;

        data[i].testData       = testData;
        data[i].testDataSize   = testDataSize;

        data[i].errorFlag      = false;
        data[i].countProcessed = 0;

        threads[i] = ::CreateThread(NULL, 0, read_thread, &data[i], 0, NULL);
    }

    ::WaitForMultipleObjects(nThreads, threads, TRUE, INFINITE);

    for (int i = 0; i < nThreads; ++i)
    {
        ::CloseHandle(threads[i]);
    }

    assert (data->fifo->GetDataSize() == 0);

    CTestResult result;

    // Validate
    uint32 countProcessed = 0;

    for (int i = 0; i < nThreads; ++i)
    {
        countProcessed += data[i].countProcessed;

        if (data[i].errorFlag)
            result.SetError(ErrorInvalidData);
    }

    if (countProcessed < fifoSize)
        result.SetError(ErrorMissingElements);

    if (countProcessed > fifoSize)
        result.SetError(ErrorAdditionalElements);

    delete[] data;
    delete[] threads;
    delete   fifo;

    result.Print();
}

static DWORD WINAPI read_thread(LPVOID lParam)
{
    thread_data* data = reinterpret_cast<thread_data*>(lParam);

    uint32 actualSize;
    const uint8* rblk;

    while ((rblk = data->fifo->Read(data->testDataSize, actualSize)))
    {
        assert (actualSize == data->testDataSize);

        for (uint32 i = 0; i < actualSize; ++i)
            if (rblk[i] != data->testData[i])
                data->errorFlag = true;

        data->countProcessed += actualSize;

        data->fifo->ReadFree(rblk, actualSize);
    }

    return EXIT_SUCCESS;
}
