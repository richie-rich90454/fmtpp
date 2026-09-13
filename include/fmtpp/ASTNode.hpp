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
     * True for RawTextNode only. Lets formatters inline single text
     * children without RTTI.
     */
    virtual bool isRawText() const { return false; }

    /**
     * True for BlockNode only. Lets suite printers accept any node
     * and branch safely without RTTI.
     */
    virtual bool isBlock() const { return false; }

    /**
     * True for IfStmtNode only. Lets elif/elsif chains nest safely.
     */
    virtual bool isIfStmt() const { return false; }

    /**
     * True for ImportDeclNode and PreprocessorDirectiveNode only. Lets
     * blank-line rules find import blocks without RTTI.
     */
    virtual bool isImportDirective() const { return false; }

    /**
     * True for ReturnStmtNode only. Lets blank-line rules find returns
     * without RTTI.
     */
    virtual bool isReturnStmt() const { return false; }

    /**
     * True for FunctionDeclNode only. Lets blank-line rules find
     * top-level definitions without RTTI.
     */
    virtual bool isFunctionDecl() const { return false; }

    /**
     * True for ClassDeclNode only. Lets blank-line rules find
     * top-level definitions without RTTI.
     */
    virtual bool isClassDecl() const { return false; }

    /**
     * Blank lines the source had before this statement (capped at 1 by
     * the parser, prettier-style). Statement-list printers re-emit one
     * blank line when this is positive, so hand-placed separators
     * survive formatting; zero keeps output compact.
     */
    int m_blankLinesBefore = 0;

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

    /**
     * Sized no-op delete for C++14 sized deallocation.
     * Individual AST nodes are freed by pool reset/destruction.
     * @param ptr Pointer to deallocate (ignored).
     * @param size Size of the allocation (ignored).
     */
    static void operator delete(void* ptr, size_t size);
};