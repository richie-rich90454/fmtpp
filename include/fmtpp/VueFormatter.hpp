#pragma once
#include "fmtpp/HtmlFormatter.hpp"

/**
 * Vue single-file-component formatter.
 * Extends HtmlFormatter: template blocks format as HTML, script blocks
 * delegate to JS/TS by their lang attribute, style blocks to CSS.
 *
 * Easter egg: Vue - progressively formatted, reactively beautiful
 */
class VueFormatter : public HtmlFormatter
{
public:
    VueFormatter(OutputBuffer& out, const Config& config);
    virtual ~VueFormatter();

    virtual void visitHtmlElement(HtmlElementNode& node);

private:
    /** Formats embedded code and splices it indented one level deeper. */
    void formatInner(const std::string& code, const std::string& language);

    /** Value of the lang attribute, or empty when absent. */
    std::string langAttr(const HtmlElementNode& node) const;
};
