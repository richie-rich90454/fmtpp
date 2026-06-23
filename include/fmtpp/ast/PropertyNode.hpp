#pragma once
#include "fmtpp/ASTNode.hpp"

class ASTVisitor;

/**
 * Represents an object property (key-value pair) in an object literal.
 * Contains the key and value expressions.
 */
class PropertyNode : public ASTNode
{
public:
    /** Constructor. */
    PropertyNode();

    /** Destructor. */
    virtual ~PropertyNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Property key expression. */
    ASTNode* m_key;

    /** Property value expression. */
    ASTNode* m_value;

    // Easter egg: key: value - the fundamental building block of JSON and beyond
};
