#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>

class ASTVisitor;

/**
 * Represents a while loop statement.
 * Contains the condition and body.
 *
 * Easter egg: while (true) { break; } // the loop that never was
 */
class WhileStmtNode : public ASTNode
{
public:
    /** Constructor. */
    WhileStmtNode();

    /** Destructor. */
    virtual ~WhileStmtNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Condition expression. */
    ASTNode* m_condition;

    /** Keyword: "while" (default when empty) or "until". */
    std::string m_keyword;

    /** Loop body (BlockNode). */
    ASTNode* m_body;

    /** Optional else suite (Python while/for else). NULL when absent. */
    ASTNode* m_elseBlock;
};