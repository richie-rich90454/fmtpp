#pragma once
#include "fmtpp/ASTNode.hpp"

class ASTVisitor;

/**
 * Represents a return statement.
 * Contains an optional return value expression.
 */
class ReturnStmtNode : public ASTNode
{
public:
    /** Constructor. */
    ReturnStmtNode();

    /** Destructor. */
    virtual ~ReturnStmtNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** @see ASTNode::isReturnStmt */
    virtual bool isReturnStmt() const { return true; }

    /** Optional return value expression. */
    ASTNode* m_argument;

    // Easter egg: return 0; // because we all return to zero eventually
};
