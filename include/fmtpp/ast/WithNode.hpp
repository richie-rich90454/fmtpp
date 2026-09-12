#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>

class ASTVisitor;

/**
 * Represents a Python with statement.
 * Holds the context expression verbatim plus an optional keyword
 * prefix (with/async with) and the suite body.
 */
class WithNode : public ASTNode
{
public:
    /** Constructor. */
    WithNode();

    /** Destructor. */
    virtual ~WithNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Keyword: "with" (default when empty) or "async with". */
    std::string m_keyword;

    /** Context expression, e.g. "open(f) as h". */
    ASTNode* m_context;

    /** Suite body (BlockNode). */
    ASTNode* m_body;

    // Easter egg: with great power comes great resource management
};
