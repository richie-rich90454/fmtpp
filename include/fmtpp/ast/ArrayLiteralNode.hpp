#pragma once
#include "fmtpp/ASTNode.hpp"
#include <vector>

class ASTVisitor;

/**
 * Represents an array literal expression.
 * Contains a list of element expressions.
 */
class ArrayLiteralNode : public ASTNode
{
public:
    /** Constructor. */
    ArrayLiteralNode();

    /** Destructor. */
    virtual ~ArrayLiteralNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Array elements. */
    std::vector<ASTNode*> m_elements;

    // Easter egg: [1, 2, 3] - the array that started it all, before off-by-one errors
};
