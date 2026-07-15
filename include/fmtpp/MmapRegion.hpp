#pragma once
#include <cstddef>

/**
 * RAII wrapper for memory-mapped file regions.
 * Uses mmap on POSIX and CreateFileMapping/MapViewOfFile on Windows.
 * Non-copyable but movable.
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

    /**
     * Move constructor.
     * Transfers ownership of the mapped region.
     * @param other Source region to move from.
     */
    MmapRegion(MmapRegion&& other);

    /**
     * Move assignment.
     * Transfers ownership of the mapped region.
     * @param other Source region to move from.
     * @return Reference to this region.
     */
    MmapRegion& operator=(MmapRegion&& other);

    /** Returns a pointer to the mapped data. */
    const void* getData() const;

    /** Returns the size of the mapped region. */
    size_t getSize() const;

    /** Returns true if the mapping was successful. */
    bool isValid() const;

private:
    // Non-copyable
    MmapRegion(const MmapRegion& other);
    MmapRegion& operator=(const MmapRegion& other);

    /** Pointer to the mapped data (NULL if mapping failed). */
    void* m_data;

    /** Size of the mapped region in bytes. */
    size_t m_size;

    /** True when the mapping is valid. */
    bool m_isValid;

#ifdef _WIN32
    /** Windows file handle (NULL on POSIX). */
    void* m_fileHandle;

    /** Windows file mapping handle (NULL on POSIX). */
    void* m_mapHandle;
#else
    /** POSIX file descriptor (-1 on Windows). */
    int m_fd;
#endif
};