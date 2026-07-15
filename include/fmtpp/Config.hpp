#pragma once
#include <string>

/**
 * Configuration for the fmtpp formatter.
 * Default values match Prettier's defaults.
 */
struct Config
{
    /** Maximum line width before wrapping. Default: 80 */
    int printWidth;

    /** Use tabs instead of spaces for indentation. Default: false */
    bool useTabs;

    /** Number of spaces per indentation level. Default: 2 */
    int tabWidth;

    /** Print semicolons at the end of statements. Default: true */
    bool semi;

    /** Use single quotes instead of double quotes. Default: false */
    bool singleQuote;

    /** Trailing comma style: "none", "es5", or "all". Default: "es5" */
    std::string trailingComma;

    /** Print spaces inside object literal braces. Default: true */
    bool bracketSpacing;

    /** Include parentheses around arrow function parameter. Default: "always" */
    std::string arrowParens;

    /** Line ending style: "lf", "crlf", "cr", or "auto". Default: "lf" */
    std::string endOfLine;

    /** Brace style: "same-line" (K&R) or "next-line" (Allman). Default: "same-line" */
    std::string braceStyle;

    /** Space before opening brace of a function. Default: true */
    bool spaceBeforeBrace;

    /** Space after keywords like if, for, while. Default: true */
    bool spaceAfterKeyword;

    /** Space around binary operators. Default: true */
    bool spaceAroundOperators;

    /** Wrap prose in markdown files. Default: "preserve" */
    std::string proseWrap;

    /** HTML whitespace sensitivity. Default: "css" */
    std::string htmlWhitespaceSensitivity;

    /** Indent script and style tags in Vue files. Default: false */
    bool vueIndentScriptAndStyle;

    /** Format embedded code in markdown/template files. Default: "auto" */
    std::string embeddedLanguageFormatting;

    /** Put each attribute on its own line in HTML/JSX. Default: false */
    bool singleAttributePerLine;

    /** Constructs a Config with default values (matching Prettier). */
    Config();

    // Easter egg: the config is the soul of the formatter
};