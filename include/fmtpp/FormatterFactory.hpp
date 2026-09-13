#pragma once
#include <string>

class OutputBuffer;
class Config;
class FormatterVisitor;

/**
 * Single home for language detection and formatter construction.
 * detectLanguage() maps a file path to a canonical language code;
 * create() builds the matching formatter (JS default for unknown codes).
 * All CLI, thread-pool and benchmark paths go through here, so a new
 * language only needs one mapping plus one case below.
 *
 * Easter egg: the factory that ends formatter-dispatch triplication
 */
class FormatterFactory
{
public:
    /**
     * Detects the canonical language code from a file extension.
     * Unknown extensions fall back to "js".
     */
    static std::string detectLanguage(const char* filepath);

    /**
     * Config-aware detection: unknown extensions fall back to
     * config.defaultLanguage, and when config.detectShebangLanguage is
     * set, a `#!` interpreter line maps to its language (python, node,
     * ruby, php only; anything else keeps the fallback). Known
     * extensions always win over the shebang.
     */
    static std::string detectLanguageForFile(const char* filepath,
        const Config& config);

    /**
     * Creates the formatter for a canonical language code.
     * Unknown codes yield a JSFormatter. Caller owns the result.
     */
    static FormatterVisitor* create(const std::string& language, OutputBuffer& out, const Config& config);

private:
    FormatterFactory();
    ~FormatterFactory();
};
