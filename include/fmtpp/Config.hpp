#pragma once
#include <map>
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

    /** Re-emit blank lines the source had between statements. Default: true */
    bool preserveBlankLines;

    /** Cap on preserved consecutive blank lines (0..9). Default: 1 */
    int maxBlankLines;

    /** Keep a blank line before the first statement. Default: false */
    bool blankLineAtFileStart;

    /** Blank line after trailing import block. Default: false */
    bool blankLineAfterImports;

    /** Blank line before return statements. Default: false */
    bool blankLineBeforeReturn;

    /** Blank lines around top-level Python def/class (0..2). Default: 0 */
    int blankLinesAroundTopLevelDefs;

    /** End file with exactly one newline. Default: true */
    bool finalNewline;

    /** Indent preserved blank lines. Default: false */
    bool indentEmptyLines;

    /** Emit a UTF-8 BOM at output start. Default: false */
    bool emitBom;

    /**
     * Function brace placement: "auto" keeps the per-language house
     * style, "same-line"/"next-line" force it. Default: "auto"
     */
    std::string braceStyleFunctions;

    /** Type (class/struct/interface/enum) brace placement. Default: "auto" */
    std::string braceStyleTypes;

    /** Space after commas. Default: true */
    bool spaceAfterComma;

    /** Space after colons (objects, annotations, CSS). Default: true */
    bool spaceAfterColon;

    /** Space between function name and paren: `f ()`. Default: false */
    bool spaceBeforeFunctionParen;

    /** Spaces inside call/parameter parens: `f( x )`. Default: false */
    bool spaceInParens;

    /** Spaces inside control-flow parens: `if ( x )`. Default: false */
    bool spaceInCondParens;

    /** Spaces inside array brackets: `[ 1 ]`. Default: false */
    bool spaceInBrackets;

    /** Spaces around `=>`. Default: true */
    bool arrowSpacing;

    /** Spaces inside `${}`: `${ x }`. Default: false */
    bool templateExprSpacing;

    /** Spaces around `=` in declarations. Default: true */
    bool spaceAroundAssignment;

    /** Only switch string quotes when escape-free. Default: true */
    bool quoteAvoidEscape;

    /** Empty blocks print `{}` inline. Default: false */
    bool emptyBlockSameLine;

    /** Outdent switch case labels (clang-format style). Default: true */
    bool indentSwitchCase;

    /**
     * `else` placement: "auto" preserves input, "always" forces a new
     * line, "never" joins with a space. Default: "auto"
     */
    std::string elseOnNewLine;

    /** `catch` placement: "auto", "always", or "never". Default: "auto" */
    std::string catchOnNewLine;

    /** `finally` placement: "auto", "always", or "never". Default: "auto" */
    std::string finallyOnNewLine;

    /** Join single-word `elseif` into `else if`. Default: true */
    bool joinElseIf;

    /** Detect language from `#!` shebang when extension is unknown. Default: true */
    bool detectShebangLanguage;

    /** Fallback language for unknown extensions. Default: "js" */
    std::string defaultLanguage;

    /** Per-language overrides: [language.<name>] tables. */
    std::map<std::string, std::map<std::string, std::string> > languageTextOverrides;

    /** Constructs a Config with default values (matching Prettier). */
    Config();

    // Easter egg: the config is the soul of the formatter
};