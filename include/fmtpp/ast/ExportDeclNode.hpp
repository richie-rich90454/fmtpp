#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>

class ASTVisitor;

/**
 * Represents an export declaration.
 * Contains the exported declaration and optional source.
 */
class ExportDeclNode : public ASTNode
{
public:
    /** Constructor. */
    ExportDeclNode();

    /** Destructor. */
    virtual ~ExportDeclNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Exported declaration. */
    ASTNode* m_declaration;

    /** Optional source module path. */
    std::string m_source;

    // Easter egg: export default 42; // the meaning of life, exported
};
