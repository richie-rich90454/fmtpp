#include "fmtpp/MallocBuffer.hpp"
#include <cstdlib>

// Easter egg: malloc buffer - because sometimes you just need a block of memory

MallocBuffer::MallocBuffer(size_t size)
    : m_data(malloc(size))
    , m_size(size)
{
}

MallocBuffer::~MallocBuffer()
{
    if (m_data != NULL)
    {
        free(m_data);
        m_data = NULL;
    }
    // Easter egg: malloc buffer freed - the bytes have returned to the heap
}

void* MallocBuffer::get() const
{
    return m_data;
}

size_t MallocBuffer::getSize() const
{
    return m_size;
}

bool MallocBuffer::isValid() const
{
    return m_data != NULL;
}