#pragma once
#include <cstddef>

/**
 * Bump-pointer memory pool for AST node allocation.
 * Allocates a single contiguous chunk of memory and returns
 * pointers by bumping an offset. No per-object deallocation.
 * The entire pool is freed in the destructor.
 *
 * Easter egg: the memory pool - where all AST nodes are born, and none are forgotten
 */
class MemoryPool
{
public:
    static const size_t DEFAULT_POOL_SIZE = 10 * 1024 * 1024; // 10 MB

    /**
     * Constructs a memory pool with the given size.
     * @param poolSize Total size of the pool in bytes (default: 10 MB).
     */
    explicit MemoryPool(size_t poolSize = DEFAULT_POOL_SIZE);

    /** Frees the entire pool. */
    ~MemoryPool();

    /**
     * Allocates memory from the pool.
     * @param bytes Number of bytes to allocate.
     * @return Pointer to allocated memory, or NULL if pool is exhausted.
     */
    void* allocate(size_t bytes);

    /** Returns the number of bytes used so far. */
    size_t getUsed() const;

    /** Returns the total pool capacity. */
    size_t getCapacity() const;

    /** Resets the pool to empty (all pointers become invalid). */
    void reset();

private:
    char* m_pool;
    size_t m_used;
    size_t m_capacity;
};

/** Global memory pool for AST node allocation. */
extern MemoryPool* g_pool;