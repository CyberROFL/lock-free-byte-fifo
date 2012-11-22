/*
    1. Написать реализацию lock-free FIFO, реализующую данный интерфейс.
    2. Написать многопоточный тест, который будет проверять, что статистически
       FIFO работает верно.

    Могут быть использованы xcode/vs/gcc make.
    В качестве результата достаточно работающего проекта,
    который можно запустить и проверить работоспособность.
*/

#ifndef SPSCBYTEFIFO_H
#define SPSCBYTEFIFO_H

typedef unsigned char uint8;
typedef unsigned int uint32;

class IByteFifo
{
public:
    virtual ~IByteFifo() {}

    /*
        @brief Request poiter for continous space of size bytes. If space of
               requested size or less is exist then return pointer to that space.
               Otherwise return NULL pointer.
        @param [in] size Desired size of continous piece.
        @param [out] writeSize Actual size of continous piece.
        @return Pointer to space if one exist, or NULL otherwise.
    */
    virtual uint8* WriteAlloc(uint32 size, uint32& writeSize) = 0;

    /*
        @brief Commit filled space so it became readable.
        @param [in] blk Pointer to memory piece returned by WriteAlloc.
        @param [in] written Actual number of bytes written in fifo.
               Can be less or equal to 'size' requested in WriteAlloc.
    */
    virtual void WriteCommit(const uint8* blk, uint32 written) = 0;

    /*
        @brief Request pointer to read continous piece of data of given size in bytes.
               If there is no piece of requested size, return piece of size less than requested if possible.
               Otherwise return NULL.
        @param [in] size Requested size of continous piece.
        @param [out] readSize Actual size of continous piece. Can be less or equal to size.
        @return Pointer to continous piece if one exist.
    */
    virtual const uint8* Read(uint32 size, uint32& readSize) = 0;

    /*
        @brief Free piece so it can be used for writting.
        @param [in] blk Pointer returned by Read
        @param [in] consumed Actual number of bytes that was cosnumed from fifo. 0 <= consumed <= 'readSize'
    */
    virtual void ReadFree(const uint8* blk, uint32 consumed) = 0;

    /*
        @return Size of data in fifo
    */
    virtual uint32 GetDataSize() const = 0;
};

#endif // SPSCBYTEFIFO_H
