#pragma once
#include <cstddef>

/**
 * RAII wrapper for memory-mapped file regions.
 * Uses mmap on POSIX and CreateFileMapping/MapViewOfFile on Windows.
 * Non-copyable.
 *
 * Easter egg: the mmap region - mapping files to memory since virtual memory was invented
 */
class MmapRegion
{
public:
    /**
     * Memory-maps a file for reading.
     * @param filepath Path to the file to map.
     */
    explicit MmapRegion(const char* filepath);

    /** Unmaps the region. */
    ~MmapRegion();

    // Non-copyable
    MmapRegion(const MmapRegion& other);
    MmapRegion& operator=(const MmapRegion& other);

    /** Returns a pointer to the mapped data. */
    const void* getData() const;

    /** Returns the size of the mapped region. */
    size_t getSize() const;

    /** Returns true if the mapping was successful. */
    bool isValid() const;

private:
    void* m_data;
    size_t m_size;
    bool m_isValid;
#ifdef _WIN32
    void* m_fileHandle;
    void* m_mapHandle;
#else
    int m_fd;
#endif
};