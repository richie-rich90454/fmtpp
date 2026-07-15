#pragma once
#include <cstddef>
#include <string>
#include <vector>

class ASTVisitor;

/**
 * Abstract base class for all AST nodes.
 * Every node implements the accept() method for the visitor pattern.
 * All nodes are allocated from the MemoryPool via placement new.
 *
 * Easter egg: the root of all syntax trees, where every parse begins
 */
class ASTNode
{
public:
    virtual ~ASTNode();

    /**
     * Accept a visitor for traversal.
     * @param visitor The ASTVisitor to accept.
     */
    virtual void accept(ASTVisitor& visitor) = 0;

    /**
     * Allocates memory for an ASTNode subclass from the global MemoryPool.
     * @param size Number of bytes to allocate.
     * @return Pointer to allocated memory, or NULL if the pool is exhausted.
     */
    static void* operator new(size_t size);

    /**
     * Placement new overload for factory-created nodes.
     * @param size Number of bytes to allocate (ignored).
     * @param ptr Pre-allocated memory from the global MemoryPool.
     * @return The same ptr value.
     */
    static void* operator new(size_t size, void* ptr);

    /**
     * No-op delete: individual AST nodes are freed by pool reset/destruction.
     * @param ptr Pointer to deallocate (ignored).
     */
    static void operator delete(void* ptr);
};