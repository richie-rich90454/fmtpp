#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>
#include <vector>

class ASTVisitor;

/**
 * Represents a class declaration.
 * Contains the class name, optional superclass, body, and implemented interfaces.
 */
class ClassDeclNode : public ASTNode
{
public:
    /** Constructor. */
    ClassDeclNode();

    /** Destructor. */
    virtual ~ClassDeclNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Class name. */
    std::string m_name;

    /** Kind word: "class" (default when empty), "struct", "union",
        "object", "trait", "contract", "@interface", "@implementation". */
    std::string m_kind;

    /** Optional superclass (IdentifierNode). */
    ASTNode* m_superClass;

    /** Class body (BlockNode). */
    ASTNode* m_body;

    /** Implemented interfaces. */
    std::vector<ASTNode*> m_implements;

    // Easter egg: class Foo extends Bar - the circle of OOP life
};
