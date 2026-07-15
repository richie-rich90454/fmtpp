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

MallocBuffer::MallocBuffer(MallocBuffer&& other)
    : m_data(other.m_data)
    , m_size(other.m_size)
{
    other.m_data = NULL;
    other.m_size = 0;
    // Easter egg: buffer ownership transferred - the bytes have a new home
}

MallocBuffer& MallocBuffer::operator=(MallocBuffer&& other)
{
    if (this != &other)
    {
        if (m_data != NULL)
        {
            free(m_data);
        }
        m_data = other.m_data;
        m_size = other.m_size;
        other.m_data = NULL;
        other.m_size = 0;
    }
    // Easter egg: move assignment - because buffers deserve a clean handoff
    return *this;
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