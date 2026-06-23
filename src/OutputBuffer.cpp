#include "fmtpp/OutputBuffer.hpp"
#include <cstdlib>
#include <cstring>
#include <cstdio>

OutputBuffer::OutputBuffer(size_t capacity)
    : m_buffer(static_cast<char*>(malloc(capacity)))
    , m_size(0)
    , m_capacity(capacity)
{
    // Easter egg: sending bits to the void, one byte at a time
    if (m_buffer != NULL)
    {
        m_buffer[0] = '\0';
    }
}

OutputBuffer::~OutputBuffer()
{
    if (m_buffer != NULL)
    {
        free(m_buffer);
        m_buffer = NULL;
    }
    // Easter egg: all bits have been sent to the void
}

void OutputBuffer::append(const char* s, size_t len)
{
    if (s == NULL || len == 0)
    {
        return;
    }
    ensureCapacity(len);
    memcpy(m_buffer + m_size, s, len);
    m_size += len;
    m_buffer[m_size] = '\0';
}

void OutputBuffer::append(const char* s)
{
    if (s == NULL)
    {
        return;
    }
    append(s, strlen(s));
}

void OutputBuffer::appendChar(char c)
{
    ensureCapacity(1);
    m_buffer[m_size] = c;
    m_size++;
    m_buffer[m_size] = '\0';
}

void OutputBuffer::printf(const char* format, ...)
{
    if (format == NULL)
    {
        return;
    }

    va_list args;
    int needed = 0;

    // First pass: determine required size
    va_start(args, format);
    needed = vsnprintf(NULL, 0, format, args);
    va_end(args);

    if (needed <= 0)
    {
        return;
    }

    // Ensure capacity for the formatted string (including null terminator)
    ensureCapacity(static_cast<size_t>(needed));

    // Second pass: write into the buffer
    va_start(args, format);
    vsnprintf(m_buffer + m_size, static_cast<size_t>(needed) + 1, format, args);
    va_end(args);

    m_size += static_cast<size_t>(needed);
    m_buffer[m_size] = '\0';
}

const char* OutputBuffer::getData() const
{
    return m_buffer;
}

size_t OutputBuffer::getSize() const
{
    return m_size;
}

size_t OutputBuffer::getCapacity() const
{
    return m_capacity;
}

std::string OutputBuffer::toString() const
{
    if (m_buffer == NULL || m_size == 0)
    {
        return std::string();
    }
    return std::string(m_buffer, m_size);
}

void OutputBuffer::clear()
{
    m_size = 0;
    if (m_buffer != NULL)
    {
        m_buffer[0] = '\0';
    }
}

void OutputBuffer::flushTo(FILE* file)
{
    if (file != NULL && m_buffer != NULL && m_size > 0)
    {
        fwrite(m_buffer, 1, m_size, file);
    }
}

void OutputBuffer::ensureCapacity(size_t additional)
{
    size_t needed = m_size + additional + 1; // +1 for null terminator
    if (needed > m_capacity)
    {
        // Double capacity until we have enough
        size_t newCapacity = m_capacity;
        if (newCapacity == 0)
        {
            newCapacity = DEFAULT_CAPACITY;
        }
        while (newCapacity < needed)
        {
            newCapacity *= 2;
        }

        char* newBuffer = static_cast<char*>(realloc(m_buffer, newCapacity));
        if (newBuffer == NULL)
        {
            // Easter egg: the void refused our bits
            // goto cleanup: not much we can do, keep the old buffer
            return;
        }

        m_buffer = newBuffer;
        m_capacity = newCapacity;
    }
}