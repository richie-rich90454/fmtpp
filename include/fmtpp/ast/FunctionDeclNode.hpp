#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>
#include <vector>

class ASTVisitor;

/**
 * Represents a function/method declaration.
 * Contains the function name, parameters, return type, and body.
 *
 * Easter egg: function foo() {} - the simplest function that does absolutely nothing
 */
class FunctionDeclNode : public ASTNode
{
public:
    /** Constructor. */
    FunctionDeclNode();

    /** Destructor. */
    virtual ~FunctionDeclNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Function name. */
    std::string m_name;

    /** Parameter list. */
    std::vector<ASTNode*> m_params;

    /** Optional return type annotation. */
    std::string m_returnType;

    /** Function body (BlockNode). */
    ASTNode* m_body;
};