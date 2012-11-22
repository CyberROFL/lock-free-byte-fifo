#ifndef TESTREAD_H
#define TESTREAD_H

#include "test_case.h"

#include <iostream>

class CReadTest : public CTestCase
{
public:
    CReadTest(uint32 fifoSize, const uint8* testData, uint32 testDataSize) :
        CTestCase(fifoSize, testData, testDataSize)
    {
    }

    const CTestResult& Run(int nThreads)
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

        CByteFifo* fifo = new CByteFifo(_fifoSize);

        assert (fifo->GetDataSize() == 0);

        // Populate
        for (uint32 i = 0; i < _fifoSize / _testDataSize; ++i)
        {
            uint32 actualSize;
            uint8* wblk;

            wblk = fifo->WriteAlloc(_testDataSize, actualSize);

            assert (!!wblk);
            assert (actualSize == _testDataSize);

            memcpy_s(wblk, actualSize, _testData, _testDataSize);

            fifo->WriteCommit(wblk, actualSize);
        }

        assert (fifo->GetDataSize() == _fifoSize);

        // Run readers
        thread_data* data = new thread_data[nThreads];

        for (int i = 0; i < nThreads; ++i)
        {
            data[i].fifo           = fifo;

            data[i].testData       = _testData;
            data[i].testDataSize   = _testDataSize;

            data[i].errorFlag      = false;
            data[i].countProcessed = 0;
        }

        RunAndWait(nThreads, ReadThread, data);

        assert (data->fifo->GetDataSize() == 0);

        // Validate
        uint32 countProcessed = 0;

        for (int i = 0; i < nThreads; ++i)
        {
            countProcessed += data[i].countProcessed;

            if (data[i].errorFlag)
                _result.SetError(ErrorInvalidData);
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
    static DWORD WINAPI ReadThread(LPVOID lParam)
    {
        thread_data* data = reinterpret_cast<thread_data*>(lParam);

        uint32 actualSize;
        const uint8* rblk;

        while ((rblk = data->fifo->Read(data->testDataSize, actualSize)))
        {
            assert (actualSize == data->testDataSize);

            for (uint32 i = 0; i < data->testDataSize; ++i)
                if (rblk[i] != data->testData[i])
                    data->errorFlag = true;

            data->countProcessed += actualSize;

            data->fifo->ReadFree(rblk, actualSize);
        }

        return EXIT_SUCCESS;
    }
};

#endif // TESTREAD_H
