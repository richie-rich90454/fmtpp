#pragma once
#include <cstddef>

/**
 * RAII wrapper for malloc'd buffers.
 * Allocates memory with malloc in the constructor and frees in the destructor.
 * Non-copyable.
 *
 * Easter egg: the malloc buffer - because sometimes you just need a block of memory
 */
class MallocBuffer
{
public:
    /**
     * Allocates a buffer of the given size.
     * @param size Number of bytes to allocate.
     */
    explicit MallocBuffer(size_t size);

    /** Frees the buffer. */
    ~MallocBuffer();

    // Non-copyable
    MallocBuffer(const MallocBuffer& other);
    MallocBuffer& operator=(const MallocBuffer& other);

    /** Returns a pointer to the allocated memory. */
    void* get() const;

    /** Returns the size of the buffer. */
    size_t getSize() const;

    /** Returns true if the allocation was successful. */
    bool isValid() const;

private:
    void* m_data;
    size_t m_size;
};