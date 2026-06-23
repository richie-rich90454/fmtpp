#include "fmtpp/CFormatter.hpp"

// Easter egg: C - the language that started it all

CFormatter::CFormatter(OutputBuffer& out, const Config& config)
    : CstyleFormatterBase(out, config)
{
}

CFormatter::~CFormatter()
{
}