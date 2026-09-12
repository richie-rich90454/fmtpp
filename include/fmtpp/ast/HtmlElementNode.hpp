#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>
#include <vector>

class ASTVisitor;

/**
 * Represents an HTML/XML element: tag name, raw attribute strings and
 * child nodes (nested elements or RawTextNode text). Used by HtmlFormatter
 * and VueFormatter (whose top level is HTML-shaped).
 */
class HtmlElementNode : public ASTNode
{
public:
    /** Constructor. */
    HtmlElementNode();

    /** Destructor. */
    virtual ~HtmlElementNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Tag name in source case (e.g. "div", "template"). */
    std::string m_tag;

    /** Raw attribute strings (e.g. "lang=\"ts\""). */
    std::vector<std::string> m_attrs;

    /** Child nodes (nested HtmlElementNode or RawTextNode). */
    std::vector<ASTNode*> m_children;

    /** True for self-closing tags (<br/>) and void elements. */
    bool m_selfClosing;

    // Easter egg: <div> everything is a box if you nest deep enough
};
