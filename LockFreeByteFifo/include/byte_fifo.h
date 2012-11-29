/*
    @author Ilnaz Nizametdinov
    @date   11/21/2012

    @description
        Lock-free implementation of byte fifo.
*/

#ifndef BYTEFIFO_H
#define BYTEFIFO_H

#include "i_byte_fifo.h"

#include <cassert>

#include <windows.h> // for Interlocked*

class CByteFifo : public IByteFifo
{
    uint32 _size;
    uint8* _data;

    // Allocated head & tail
    uint8* volatile _head_alloc;
    uint8* volatile _tail_alloc;

    // Commited head & tail
    uint8* volatile _head;
    uint8* volatile _tail;

public:
    CByteFifo(uint32 size) :

        _size(size),
        _data(new uint8[size + 1]) // +1 extra for slack
    {
        _head_alloc = &_data[0];
        _tail_alloc = &_data[0];

        _head = &_data[0];
        _tail = &_data[0];
    }

    ~CByteFifo()
    {
        delete[] _data;
    }

    uint8* WriteAlloc(uint32 size, uint32& writeSize)
    {
        assert (size > 0); // Must be not null

        writeSize = (size > _size) ? _size : size;

        uint8* old_tail_alloc = _tail_alloc;
        while (true)
        {
            uint32 availableSize;

            if (increase(old_tail_alloc) == _head_alloc)
                return NULL; // Buffer is full

            else if (old_tail_alloc == &_data[_size])
                availableSize = 1;

            else if (_head_alloc <= old_tail_alloc)
                availableSize = (&_data[_size] - old_tail_alloc);

            else // old_tail_alloc < _head_alloc
                availableSize = _head_alloc - old_tail_alloc - 1;

            if (writeSize > availableSize)
                writeSize = availableSize;

            uint8* initial_tail_alloc =
                cas(&_tail_alloc, old_tail_alloc, increase(old_tail_alloc, writeSize));

            if (initial_tail_alloc == old_tail_alloc)
                return old_tail_alloc; // Return pointer to allocated space

            old_tail_alloc = initial_tail_alloc;
        }

        return NULL;
    }

    void WriteCommit(const uint8* blk, uint32 written)
    {
        assert (written <= _size); // Must be less than fifo size

        uint32 spin_count = 1;
        while (blk != _tail)
        {
            if (!(spin_count++ % 50))
                ::Sleep(1); // Give a time for another thread
            else
                ; // nop
        }

        _tail = increase(const_cast<uint8*>(blk), written);
    }

    const uint8* Read(uint32 size, uint32& readSize)
    {
        assert (size > 0); // Must be not null

        readSize = (size > _size) ? _size : size;

        uint8* old_head = _head;
        while (true)
        {
            uint32 availableSize;

            if (old_head == _tail)
                return NULL; // Buffer is empty

            else if (old_head == &_data[_size])
                availableSize = 1;

            else if (old_head < _tail)
                availableSize = (_tail - old_head);

            else // _tail < old_head
                availableSize = (&_data[_size] - old_head + 1);

            if (readSize > availableSize)
                readSize = availableSize;

            uint8* initial_head =
                cas(&_head, old_head, increase(old_head, readSize));

            if (initial_head == old_head)
                return old_head; // Return pointer to continous piece of data

            old_head = initial_head;
        }

        return NULL;
    }

    void ReadFree(const uint8* blk, uint32 consumed)
    {
        assert (consumed <= _size); // Must be less than fifo size

        uint32 spin_count = 1;
        while (blk != _head_alloc)
        {
            if (!(spin_count++ % 50))
                ::Sleep(1); // Give a time for another thread
            else
                ; // nop
        }

        _head_alloc = increase(const_cast<uint8*>(blk), consumed);
    }

    uint32 GetDataSize() const
    {
        uint8* old_head = _head;
        uint8* old_tail = _tail;

        if (old_tail >= old_head)
            return (old_tail - old_head);
        else
            return (_size - (old_head - old_tail - 1));
    }

private:
    template<class Ptr>
    inline Ptr cas(Ptr volatile* dst, Ptr cmp, Ptr exch)
    {
        return reinterpret_cast<Ptr>(
                   ::InterlockedCompareExchangePointer(
                       reinterpret_cast<PVOID volatile*>(dst), exch, cmp
                   )
               );
    }

    inline uint8* increase(uint8* ptr, uint32 inc = 1)
    {
        if ((ptr + inc) <= &_data[_size])
            return (ptr + inc);
        else
            return &_data[0];
    }
};

#endif // BYTEFIFO_H
