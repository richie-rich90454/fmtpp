#pragma once
#include "fmtpp/ASTNode.hpp"

class ASTVisitor;

/**
 * Represents a for/for-in/for-of loop statement.
 * Contains the init, condition, update, and body.
 *
 * Easter egg: for (;;) {} // the infinite loop that keeps on giving
 */
class ForStmtNode : public ASTNode
{
public:
    /** Constructor. */
    ForStmtNode();

    /** Destructor. */
    virtual ~ForStmtNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Initialization expression (e.g., let i = 0). */
    ASTNode* m_init;

    /** Condition expression. */
    ASTNode* m_condition;

    /** Update expression (e.g., i++). */
    ASTNode* m_update;

    /** Loop keyword: "for" (default when empty), "foreach". */
    std::string m_keyword;

    /** True for C-style (init; cond; update) headers, false for raw
        single headers such as for-in, foreach and range loops. */
    bool m_classic;

    /** Loop body (BlockNode). */
    ASTNode* m_body;
};