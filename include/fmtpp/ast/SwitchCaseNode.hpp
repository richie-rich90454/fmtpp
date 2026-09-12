#pragma once
#include "fmtpp/ASTNode.hpp"

class ASTVisitor;

/**
 * Represents one case arm of an indentation-style match statement
 * (Python match/case, Ruby case/when with bodies). The pattern prints
 * at arm level, the body suite one level deeper.
 */
class SwitchCaseNode : public ASTNode
{
public:
    /** Constructor. */
    SwitchCaseNode();

    /** Destructor. */
    virtual ~SwitchCaseNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Arm pattern (RawText, e.g. "case 1:" or "when x"). */
    ASTNode* m_pattern;

    /** Arm body suite (BlockNode, may be NULL). */
    ASTNode* m_body;

    // Easter egg: in case of beautiful formatting, break nothing
};
