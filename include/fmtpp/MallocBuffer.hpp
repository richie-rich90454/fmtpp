#pragma once
#include <cstddef>

/**
 * RAII wrapper for malloc'd buffers.
 * Allocates memory with malloc in the constructor and frees in the destructor.
 * Non-copyable but movable.
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

    /**
     * Move constructor.
     * Transfers ownership of the allocated buffer.
     * @param other Source buffer to move from.
     */
    MallocBuffer(MallocBuffer&& other);

    /**
     * Move assignment.
     * Transfers ownership of the allocated buffer.
     * @param other Source buffer to move from.
     * @return Reference to this buffer.
     */
    MallocBuffer& operator=(MallocBuffer&& other);

    /** Returns a pointer to the allocated memory. */
    void* get() const;

    /** Returns the size of the buffer. */
    size_t getSize() const;

    /** Returns true if the allocation was successful. */
    bool isValid() const;

private:
    // Non-copyable
    MallocBuffer(const MallocBuffer& other);
    MallocBuffer& operator=(const MallocBuffer& other);

    /** Pointer to the malloc'd memory (NULL if allocation failed). */
    void* m_data;

    /** Size of the allocated block in bytes. */
    size_t m_size;
};