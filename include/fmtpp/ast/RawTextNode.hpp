#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>

class ASTVisitor;

/**
 * Verbatim source slice the parser understood well enough not to touch
 * but not well enough to restructure (e.g. labels, lone break/continue,
 * exotic headers). Formatters print it exactly as-is, so it can never
 * corrupt code — the worst case is a missed re-indent, never a rewrite.
 */
class RawTextNode : public ASTNode
{
public:
    /** Constructor. */
    RawTextNode();

    /** Destructor. */
    virtual ~RawTextNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** @see ASTNode::isRawText */
    virtual bool isRawText() const { return true; }

    /** Exact source text, without leading indent or trailing newline. */
    std::string m_text;

    // Easter egg: raw text - the parser's way of saying "you deal with it"
};
