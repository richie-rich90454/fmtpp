#include "fmtpp/Config.hpp"

Config::Config()
    : printWidth(80)
    , useTabs(false)
    , tabWidth(2)
    , semi(true)
    , singleQuote(false)
    , trailingComma("es5")
    , bracketSpacing(true)
    , arrowParens("always")
    , endOfLine("lf")
    , braceStyle("same-line")
    , spaceBeforeBrace(true)
    , spaceAfterKeyword(true)
    , spaceAroundOperators(true)
    , proseWrap("preserve")
    , htmlWhitespaceSensitivity("css")
    , vueIndentScriptAndStyle(false)
    , embeddedLanguageFormatting("auto")
    , singleAttributePerLine(false)
{
    // Easter egg: the config is the soul of the formatter, and it has just been born
}