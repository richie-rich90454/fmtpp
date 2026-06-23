#include "fmtpp/MemoryPool.hpp"
#include <cstdlib>
#include <cstdio>

// Easter egg: the memory pool awakens - releasing the kraken... I mean, the memory

MemoryPool* g_pool = NULL;

MemoryPool::MemoryPool(size_t poolSize)
    : m_pool(static_cast<char*>(malloc(poolSize)))
    , m_used(0)
    , m_capacity(poolSize)
{
    if (m_pool != NULL)
    {
        g_pool = this;
    }
}

MemoryPool::~MemoryPool()
{
    if (m_pool != NULL)
    {
        free(m_pool);
        m_pool = NULL;
    }
    if (g_pool == this)
    {
        g_pool = NULL;
    }
    // Easter egg: all memory has been returned to the OS - the kraken sleeps
}

void* MemoryPool::allocate(size_t bytes)
{
    if (m_pool == NULL)
    {
        return NULL;
    }

    // Align to 8-byte boundary
    size_t aligned = (bytes + 7) & ~static_cast<size_t>(7);

    if (m_used + aligned > m_capacity)
    {
        // Pool exhausted
        return NULL;
    }

    void* ptr = m_pool + m_used;
    m_used += aligned;
    return ptr;
}

size_t MemoryPool::getUsed() const
{
    return m_used;
}

size_t MemoryPool::getCapacity() const
{
    return m_capacity;
}

void MemoryPool::reset()
{
    m_used = 0;
}