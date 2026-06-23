#pragma once
#include "fmtpp/CstyleFormatterBase.hpp"

/**
 * C language code formatter.
 * Extends CstyleFormatterBase with no additional overrides
 * (C uses the base C-style formatting directly).
 * C doesn't have classes, namespaces, or templates.
 *
 * Easter egg: C - the language that started it all, still going strong
 */
class CFormatter : public CstyleFormatterBase
{
public:
    CFormatter(OutputBuffer& out, const Config& config);
    virtual ~CFormatter();
};