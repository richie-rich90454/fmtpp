#pragma once
#include "fmtpp/ASTNode.hpp"
#include <vector>
#include <string>

class ASTVisitor;

/**
 * Represents an arrow function expression.
 * Contains parameters, optional return type, and function body.
 */
class ArrowFunctionNode : public ASTNode
{
public:
    /** Constructor. */
    ArrowFunctionNode();

    /** Destructor. */
    virtual ~ArrowFunctionNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Parameter list. */
    std::vector<ASTNode*> m_params;

    /** Optional return type annotation. */
    std::string m_returnType;

    /** Function body (BlockNode or expression). */
    ASTNode* m_body;

    // Easter egg: () => { } - the arrow that flew straight into every JavaScript developer's heart
};
