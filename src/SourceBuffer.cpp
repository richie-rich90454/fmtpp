#include "fmtpp/SourceBuffer.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Easter egg: the source buffer - loading code from disk, one file at a time

SourceBuffer::SourceBuffer(const char* filepath)
    : m_data(NULL)
    , m_size(0)
    , m_isValid(false)
    , m_isMapped(false)
{
    FILE* file = NULL;
    long fileSize = 0;
    size_t bytesRead = 0;

    if (filepath == NULL)
    {
        return;
    }

    file = fopen(filepath, "rb");
    if (file == NULL)
    {
        return;
    }

    // Get file size
    if (fseek(file, 0, SEEK_END) != 0)
    {
        goto cleanup;
    }
    fileSize = ftell(file);
    if (fileSize < 0)
    {
        goto cleanup;
    }
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        goto cleanup;
    }

    m_size = static_cast<size_t>(fileSize);
    if (m_size == 0)
    {
        // Empty file is valid
        m_data = static_cast<char*>(malloc(1));
        if (m_data != NULL)
        {
            m_data[0] = '\0';
            m_isValid = true;
        }
        goto cleanup;
    }

    m_data = static_cast<char*>(malloc(m_size + 1));
    if (m_data == NULL)
    {
        goto cleanup;
    }

    bytesRead = fread(m_data, 1, m_size, file);
    if (bytesRead != m_size)
    {
        free(m_data);
        m_data = NULL;
        m_size = 0;
        goto cleanup;
    }

    m_data[m_size] = '\0';
    m_isValid = true;

cleanup:
    if (file != NULL)
    {
        fclose(file);
        file = NULL;
    }
    // Easter egg: source file loaded - the code is now in memory, ready for transformation
}

SourceBuffer::~SourceBuffer()
{
    if (m_data != NULL)
    {
        free(m_data);
        m_data = NULL;
    }
    // Easter egg: source buffer freed - the code has been processed
}

const char* SourceBuffer::getData() const
{
    return m_data;
}

size_t SourceBuffer::getSize() const
{
    return m_size;
}

bool SourceBuffer::isValid() const
{
    return m_isValid;
}