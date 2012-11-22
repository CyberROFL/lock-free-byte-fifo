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
    CReadTest  readTest (fifoSize, testData, testDataSize);
    CWriteTest writeTest(fifoSize, testData, testDataSize);

    readTest. Run(nReaders).Print();
    writeTest.Run(nWriters).Print();

    return EXIT_SUCCESS;
}
