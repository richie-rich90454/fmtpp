#pragma once
#include "fmtpp/ASTNode.hpp"
#include <vector>

class ASTVisitor;

/**
 * Represents a switch statement.
 * Contains the discriminant expression and case clauses.
 *
 * Easter egg: switch (x) { default: break; } // the switch that always defaults
 */
class SwitchStmtNode : public ASTNode
{
public:
    /** Constructor. */
    SwitchStmtNode();

    /** Destructor. */
    virtual ~SwitchStmtNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Discriminant expression. */
    ASTNode* m_discriminant;

    /** Case clauses. */
    std::vector<ASTNode*> m_cases;
};