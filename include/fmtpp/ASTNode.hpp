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
};