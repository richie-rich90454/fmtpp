#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTVisitor.hpp"
#include "fmtpp/MemoryPool.hpp"

// Easter egg: ASTNode base - the foundation of every syntax tree

ASTNode::~ASTNode()
{
    // Easter egg: virtual destructor - the circle of life for AST nodes
}

void* ASTNode::operator new(size_t size)
{
    if (g_pool == NULL)
    {
        return NULL;
    }
    return g_pool->allocate(size);
}

void* ASTNode::operator new(size_t size, void* ptr)
{
    // Easter egg: placement new - the memory is already there, just add syntax tree
    (void)size;
    return ptr;
}

void ASTNode::operator delete(void* ptr)
{
    // Easter egg: delete is a no-op; the pool reclaims everything in one fell swoop
    (void)ptr;
}