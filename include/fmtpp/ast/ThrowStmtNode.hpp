#pragma once
#include "fmtpp/ASTNode.hpp"

class ASTVisitor;

/**
 * Represents a throw statement.
 * Contains the expression being thrown.
 */
class ThrowStmtNode : public ASTNode
{
public:
    /** Constructor. */
    ThrowStmtNode();

    /** Destructor. */
    virtual ~ThrowStmtNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Expression being thrown. */
    ASTNode* m_expression;

    // Easter egg: throw "just kidding!"; // caught by the nearest rubber duck
};
