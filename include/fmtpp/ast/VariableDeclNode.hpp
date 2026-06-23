#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>

class ASTVisitor;

/**
 * Represents a variable declaration.
 * Contains the kind (let/const/var), name, type annotation, and initializer.
 *
 * Easter egg: let x = 42; // the answer to everything, now in a variable
 */
class VariableDeclNode : public ASTNode
{
public:
    /** Constructor. */
    VariableDeclNode();

    /** Destructor. */
    virtual ~VariableDeclNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Declaration kind (e.g., "let", "const", "var", "int", "auto"). */
    std::string m_kind;

    /** Variable name. */
    std::string m_name;

    /** Optional type annotation. */
    std::string m_type;

    /** Optional initializer expression. */
    ASTNode* m_initializer;
};