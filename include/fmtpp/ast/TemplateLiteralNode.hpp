#pragma once
#include "fmtpp/ASTNode.hpp"
#include <vector>

class ASTVisitor;

/**
 * Represents a template literal expression.
 * Contains quasis (static string parts) and expressions (interpolated values).
 */
class TemplateLiteralNode : public ASTNode
{
public:
    /** Constructor. */
    TemplateLiteralNode();

    /** Destructor. */
    virtual ~TemplateLiteralNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Static string parts (quasis). */
    std::vector<ASTNode*> m_quasis;

    /** Interpolated expressions. */
    std::vector<ASTNode*> m_expressions;

    // Easter egg: `Hello, ${name}!` - backtick magic that makes strings dynamic
};
