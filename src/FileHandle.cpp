#include "fmtpp/FileHandle.hpp"

// Easter egg: the file handle - guarding files since C was invented

FileHandle::FileHandle(const char* filepath, const char* mode)
    : m_file(NULL)
{
    if (filepath != NULL && mode != NULL)
    {
        m_file = fopen(filepath, mode);
    }
}

FileHandle::~FileHandle()
{
    if (m_file != NULL)
    {
        fclose(m_file);
        m_file = NULL;
    }
    // Easter egg: file handle closed - the bits are safe on disk
}

FileHandle::FileHandle(FileHandle&& other)
    : m_file(other.m_file)
{
    other.m_file = NULL;
    // Easter egg: ownership transferred - the file has a new guardian
}

FileHandle& FileHandle::operator=(FileHandle&& other)
{
    if (this != &other)
    {
        if (m_file != NULL)
        {
            fclose(m_file);
        }
        m_file = other.m_file;
        other.m_file = NULL;
    }
    // Easter egg: move assignment - because files deserve a clean handoff
    return *this;
}

FILE* FileHandle::get() const
{
    return m_file;
}

bool FileHandle::isValid() const
{
    return m_file != NULL;
}