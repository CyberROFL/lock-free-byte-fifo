#include "stdafx.h"

#include "write_test.h"
#include "thread_data.h"
#include "test_result.h"

#include <iostream>
#include <cassert>

static DWORD WINAPI write_thread(LPVOID lParam);

void write_test(int nThreads,
                uint32 fifoSize,
                const uint8* testData,
                uint32 testDataSize)
{
    /*
        We create a single fifo with 'fifoSize' elements.
        The fifo starts empty.

        We create 'nThreads' threads, where each thread busy-works writing.

        The user data in each written element is a combination
        of the thread number and the 'testData'.

        After the threads are complete, we validate by
        checking the user data on a per thread basis.
    */

    std::cout << "Write test:\t";

    assert (!(fifoSize % testDataSize));

    CByteFifo* fifo = new CByteFifo(fifoSize);

    assert (fifo->GetDataSize() == 0);

    // Run writers
    thread_data* data = new thread_data[nThreads];
    HANDLE* threads   = new HANDLE[nThreads];

    for (int i = 0; i < nThreads; ++i)
    {
        data[i].fifo           = fifo;

        data[i].testData       = testData;
        data[i].testDataSize   = testDataSize;

        data[i].errorFlag      = false;
        data[i].countProcessed = 0;

        data[i].threadId       = i;

        threads[i] = ::CreateThread(NULL, 0, write_thread, &data[i], 0, NULL);
    }

    ::WaitForMultipleObjects(nThreads, threads, TRUE, INFINITE);

    for (int i = 0; i < nThreads; ++i)
    {
        ::CloseHandle(threads[i]);
    }

    assert (fifo->GetDataSize() == fifoSize);

    CTestResult result;

    // Validate
    for (uint32 i = 0; i < fifoSize / testDataSize; ++i)
    {
        uint32 actualSize;
        const uint8* rblk;

        rblk = fifo->Read(testDataSize, actualSize);

        assert (!!rblk);
        assert (actualSize == testDataSize);

        uint32 threadId = (rblk[0] - testData[0]);
        for (uint32 i = 1; i < actualSize; ++i)
        {
            if ((rblk[i] - testData[i]) != threadId)
                result.SetError(ErrorInvalidData);
        }

        fifo->ReadFree(rblk, actualSize);
    }

    assert (fifo->GetDataSize() == 0);

    uint32 countProcessed = 0;

    for (int i = 0; i < nThreads; ++i)
    {
        countProcessed += data[i].countProcessed;
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

static DWORD WINAPI write_thread(LPVOID lParam)
{
    thread_data* data = reinterpret_cast<thread_data*>(lParam);

    uint32 actualSize;
    uint8* wblk;

    while ((wblk = data->fifo->WriteAlloc(data->testDataSize, actualSize)))
    {
        assert (actualSize == data->testDataSize);

        uint8* testData = new uint8[actualSize];
        for (uint32 i = 0; i < actualSize; ++i)
            testData[i] = data->testData[i] + data->threadId;

        memcpy_s(wblk, actualSize, testData, data->testDataSize);

        data->countProcessed += actualSize;

        data->fifo->WriteCommit(wblk, actualSize);

        delete[] testData;
    }

    return EXIT_SUCCESS;
}
