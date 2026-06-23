#pragma once
#include "fmtpp/ASTNode.hpp"
#include <vector>

class ASTVisitor;

/**
 * Represents an object literal expression.
 * Contains a list of key-value property pairs.
 */
class ObjectLiteralNode : public ASTNode
{
public:
    /** Constructor. */
    ObjectLiteralNode();

    /** Destructor. */
    virtual ~ObjectLiteralNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Object properties (key-value pairs). */
    std::vector<ASTNode*> m_properties;

    // Easter egg: { key: "value" } - the JSON-esque heart of JavaScript
};
