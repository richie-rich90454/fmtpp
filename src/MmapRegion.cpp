#include "fmtpp/MmapRegion.hpp"
#include <cstdio>

// Easter egg: memory mapping - the OS's way of saying "here, have some memory"

#ifdef _WIN32
#include <windows.h>

MmapRegion::MmapRegion(const char* filepath)
    : m_data(NULL)
    , m_size(0)
    , m_isValid(false)
    , m_fileHandle(NULL)
    , m_mapHandle(NULL)
{
    if (filepath == NULL) { return; }

    HANDLE hFile = CreateFileA(
        filepath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) { return; }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) { CloseHandle(hFile); return; }

    HANDLE hMap = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMap == NULL) { CloseHandle(hFile); return; }

    void* view = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (view == NULL) { CloseHandle(hMap); CloseHandle(hFile); return; }

    m_data = view;
    m_size = static_cast<size_t>(fileSize);
    m_fileHandle = hFile;
    m_mapHandle = hMap;
    m_isValid = true;
}

MmapRegion::~MmapRegion()
{
    if (m_data != NULL)
    {
        UnmapViewOfFile(m_data);
        m_data = NULL;
    }
    if (m_mapHandle != NULL)
    {
        CloseHandle(m_mapHandle);
        m_mapHandle = NULL;
    }
    if (m_fileHandle != NULL)
    {
        CloseHandle(m_fileHandle);
        m_fileHandle = NULL;
    }
    // Easter egg: memory unmapped - the OS can have its pages back
}

MmapRegion::MmapRegion(MmapRegion&& other)
    : m_data(other.m_data)
    , m_size(other.m_size)
    , m_isValid(other.m_isValid)
    , m_fileHandle(other.m_fileHandle)
    , m_mapHandle(other.m_mapHandle)
{
    other.m_data = NULL;
    other.m_size = 0;
    other.m_isValid = false;
    other.m_fileHandle = NULL;
    other.m_mapHandle = NULL;
    // Easter egg: region ownership transferred - the pages have a new caretaker
}

MmapRegion& MmapRegion::operator=(MmapRegion&& other)
{
    if (this != &other)
    {
        if (m_data != NULL)
        {
            UnmapViewOfFile(m_data);
        }
        if (m_mapHandle != NULL)
        {
            CloseHandle(m_mapHandle);
        }
        if (m_fileHandle != NULL)
        {
            CloseHandle(m_fileHandle);
        }
        m_data = other.m_data;
        m_size = other.m_size;
        m_isValid = other.m_isValid;
        m_fileHandle = other.m_fileHandle;
        m_mapHandle = other.m_mapHandle;
        other.m_data = NULL;
        other.m_size = 0;
        other.m_isValid = false;
        other.m_fileHandle = NULL;
        other.m_mapHandle = NULL;
    }
    // Easter egg: move assignment - mapped memory gets a new address book
    return *this;
}

#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

MmapRegion::MmapRegion(const char* filepath)
    : m_data(NULL)
    , m_size(0)
    , m_isValid(false)
    , m_fd(-1)
{
    if (filepath == NULL) { return; }

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) { return; }

    struct stat st;
    if (fstat(fd, &st) < 0) { close(fd); return; }

    size_t fileSize = static_cast<size_t>(st.st_size);
    if (fileSize == 0) { close(fd); return; }

    void* mapped = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) { close(fd); return; }

    m_data = mapped;
    m_size = fileSize;
    m_fd = fd;
    m_isValid = true;
}

MmapRegion::~MmapRegion()
{
    if (m_data != NULL)
    {
        munmap(m_data, m_size);
        m_data = NULL;
    }
    if (m_fd >= 0)
    {
        close(m_fd);
        m_fd = -1;
    }
    // Easter egg: memory unmapped - the OS can have its pages back
}

MmapRegion::MmapRegion(MmapRegion&& other)
    : m_data(other.m_data)
    , m_size(other.m_size)
    , m_isValid(other.m_isValid)
    , m_fd(other.m_fd)
{
    other.m_data = NULL;
    other.m_size = 0;
    other.m_isValid = false;
    other.m_fd = -1;
    // Easter egg: region ownership transferred - the pages have a new caretaker
}

MmapRegion& MmapRegion::operator=(MmapRegion&& other)
{
    if (this != &other)
    {
        if (m_data != NULL)
        {
            munmap(m_data, m_size);
        }
        if (m_fd >= 0)
        {
            close(m_fd);
        }
        m_data = other.m_data;
        m_size = other.m_size;
        m_isValid = other.m_isValid;
        m_fd = other.m_fd;
        other.m_data = NULL;
        other.m_size = 0;
        other.m_isValid = false;
        other.m_fd = -1;
    }
    // Easter egg: move assignment - mapped memory gets a new address book
    return *this;
}
#endif

const void* MmapRegion::getData() const
{
    return m_data;
}

size_t MmapRegion::getSize() const
{
    return m_size;
}

bool MmapRegion::isValid() const
{
    return m_isValid;
}