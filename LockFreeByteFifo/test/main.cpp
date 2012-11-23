#include "stdafx.h"

#include "read_test.h"
#include "write_test.h"

int _tmain(int argc, _TCHAR* argv[])
{
    // Test-case settings
    const uint32 fifoSize        = 30000;

    const int nReaders           = 4;
    const int nWriters           = 4;

    const uint32 testDataSize    = 4;
    uint8 testData[testDataSize] = { 'a', 'b', 'c', 'd' };

    // Test
    read_test (nReaders, fifoSize, testData, testDataSize);
    write_test(nWriters, fifoSize, testData, testDataSize);

    return EXIT_SUCCESS;
}
