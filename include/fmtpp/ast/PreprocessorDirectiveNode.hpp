#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>

class ASTVisitor;

/**
 * Represents a C/C++ preprocessor directive.
 * Contains the directive text (e.g., "#define", "#include").
 */
class PreprocessorDirectiveNode : public ASTNode
{
public:
    /** Constructor. */
    PreprocessorDirectiveNode();

    /** Destructor. */
    virtual ~PreprocessorDirectiveNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** @see ASTNode::isImportDirective */
    virtual bool isImportDirective() const { return true; }

    /** Directive string (e.g., "#define FOO 42"). */
    std::string m_directive;

    // Easter egg: #define TRUE FALSE // happy debugging!
};
