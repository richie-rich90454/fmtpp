#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>

class ASTVisitor;

/**
 * Represents an if/else if/else statement.
 * Contains the condition, then-branch, and optional else-branch.
 *
 * Easter egg: if (true) { } else { } // the most useless branch ever
 */
class IfStmtNode : public ASTNode
{
public:
    /** Constructor. */
    IfStmtNode();

    /** Destructor. */
    virtual ~IfStmtNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** @see ASTNode::isIfStmt */
    virtual bool isIfStmt() const { return true; }

    /** Condition expression. */
    ASTNode* m_condition;

    /** Keyword: "if" (default when empty) or "unless". */
    std::string m_keyword;

    /** Then-branch (BlockNode or statement). */
    ASTNode* m_thenBranch;

    /** Optional else-branch (BlockNode or IfStmtNode). */
    ASTNode* m_elseBranch;
};