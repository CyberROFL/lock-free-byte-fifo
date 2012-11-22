#ifndef TESTWRITE_H
#define TESTWRITE_H

#include "test_case.h"

#include <iostream>

class CWriteTest : public CTestCase
{
public:
    CWriteTest(uint32 fifoSize, const uint8* testData, uint32 testDataSize) :
        CTestCase(fifoSize, testData, testDataSize)
    {
    }

    const CTestResult& Run(int nThreads)
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

        CByteFifo* fifo = new CByteFifo(_fifoSize);

        assert (fifo->GetDataSize() == 0);

        // Run writers
        thread_data* data = new thread_data[nThreads];

        for (int i = 0; i < nThreads; ++i)
        {
            data[i].fifo           = fifo;

            data[i].testData       = _testData;
            data[i].testDataSize   = _testDataSize;

            data[i].errorFlag      = false;
            data[i].countProcessed = 0;

            data[i].threadId       = i;
        }

        RunAndWait(nThreads, WriteThread, data);

        assert (fifo->GetDataSize() == _fifoSize);

        // Validate
        for (uint32 i = 0; i < _fifoSize / _testDataSize; ++i)
        {
            uint32 actualSize;
            const uint8* rblk;

            rblk = fifo->Read(_testDataSize, actualSize);

            assert (!!rblk);
            assert (actualSize == _testDataSize);

            uint32 threadId = (rblk[0] - _testData[0]);
            for (uint32 i = 1; i < _testDataSize; ++i)
            {
                if ((rblk[i] - _testData[i]) != threadId)
                    _result.SetError(ErrorInvalidData);
            }

            fifo->ReadFree(rblk, actualSize);
        }

        assert (fifo->GetDataSize() == 0);

        uint32 countProcessed = 0;

        for (int i = 0; i < nThreads; ++i)
        {
            countProcessed += data[i].countProcessed;
        }

        if (countProcessed < _fifoSize)
            _result.SetError(ErrorMissingElements);

        if (countProcessed > _fifoSize)
            _result.SetError(ErrorAdditionalElements);

        delete[] data;
        delete   fifo;

        return _result;
    }

private:
    static DWORD WINAPI WriteThread(LPVOID lParam)
    {
        thread_data* data = reinterpret_cast<thread_data*>(lParam);

        uint32 actualSize;
        uint8* wblk;

        while ((wblk = data->fifo->WriteAlloc(data->testDataSize, actualSize)))
        {
            assert (actualSize == data->testDataSize);

            uint8* testData = new uint8[data->testDataSize];
            for (uint32 i = 0; i < data->testDataSize; ++i)
                testData[i] = data->testData[i] + data->threadId;

            memcpy_s(wblk, actualSize, testData, data->testDataSize);

            data->countProcessed += actualSize;

            data->fifo->WriteCommit(wblk, actualSize);

            delete[] testData;
        }

        return EXIT_SUCCESS;
    }
};

#endif // TESTWRITE_H
