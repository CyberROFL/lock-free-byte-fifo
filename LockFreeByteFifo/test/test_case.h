#ifndef TESTCASE_H
#define TESTCASE_H

#include "byte_fifo.h"
#include "test_result.h"

#include <cassert>

class CTestCase
{
protected:
    uint32       _fifoSize;
    const uint8* _testData;
    uint32       _testDataSize;

    struct thread_data
    {
        CByteFifo* fifo;

        const uint8* testData;
        uint32 testDataSize;

        bool errorFlag;
        uint32 countProcessed;

        uint32 threadId; // Not user for read test
    };

    CTestResult _result;

public:
    CTestCase(uint32 fifoSize, const uint8* testData, uint32 testDataSize) :

        _fifoSize    (fifoSize),
        _testData    (testData),
        _testDataSize(testDataSize)
    {
        assert (!(fifoSize % testDataSize));
    }

    virtual const CTestResult& Run(int nThreads) = 0;

protected:
    void RunAndWait(int nThreads,
                    LPTHREAD_START_ROUTINE lpStartAddress,
                    thread_data* data)
    {
        HANDLE* threads = new HANDLE[nThreads];

        for (int i = 0; i < nThreads; ++i)
        {
            threads[i] = ::CreateThread(NULL, 0, lpStartAddress, &data[i], 0, NULL);
        }

        ::WaitForMultipleObjects(nThreads, threads, TRUE, INFINITE);

        for (int i = 0; i < nThreads; ++i)
        {
            ::CloseHandle(threads[i]);
        }

        delete[] threads;
    }
};

#endif // TESTCASE_H
