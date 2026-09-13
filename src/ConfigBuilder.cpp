#include "fmtpp/ConfigBuilder.hpp"
#include <toml++/toml.hpp>
#include <cstdio>
#include <optional>

// Easter egg: building configs, one TOML at a time

Config ConfigBuilder::buildFromFile(const std::string& filepath)
{
    Config config;
    toml::table table;
    try
    {
        table = toml::parse_file(filepath);
    }
    catch (const toml::parse_error& err)
    {
        // Easter egg: a broken TOML is still a TOML at heart
        (void)err;
        return config;
    }

    if (auto node = table.get("printWidth"))
    {
        if (auto value = node->value<int64_t>())
        {
            config.printWidth = static_cast<int>(*value);
        }
    }

    if (auto node = table.get("useTabs"))
    {
        if (auto value = node->value<bool>())
        {
            config.useTabs = *value;
        }
    }

    if (auto node = table.get("tabWidth"))
    {
        if (auto value = node->value<int64_t>())
        {
            config.tabWidth = static_cast<int>(*value);
        }
    }

    if (auto node = table.get("semi"))
    {
        if (auto value = node->value<bool>())
        {
            config.semi = *value;
        }
    }

    if (auto node = table.get("singleQuote"))
    {
        if (auto value = node->value<bool>())
        {
            config.singleQuote = *value;
        }
    }

    if (auto node = table.get("trailingComma"))
    {
        if (auto value = node->value<std::string>())
        {
            config.trailingComma = *value;
        }
    }

    if (auto node = table.get("bracketSpacing"))
    {
        if (auto value = node->value<bool>())
        {
            config.bracketSpacing = *value;
        }
    }

    if (auto node = table.get("arrowParens"))
    {
        if (auto value = node->value<std::string>())
        {
            config.arrowParens = *value;
        }
    }

    if (auto node = table.get("endOfLine"))
    {
        if (auto value = node->value<std::string>())
        {
            config.endOfLine = *value;
        }
    }

    if (auto node = table.get("braceStyle"))
    {
        if (auto value = node->value<std::string>())
        {
            config.braceStyle = *value;
        }
    }

    if (auto node = table.get("spaceBeforeBrace"))
    {
        if (auto value = node->value<bool>())
        {
            config.spaceBeforeBrace = *value;
        }
    }

    if (auto node = table.get("spaceAfterKeyword"))
    {
        if (auto value = node->value<bool>())
        {
            config.spaceAfterKeyword = *value;
        }
    }

    if (auto node = table.get("spaceAroundOperators"))
    {
        if (auto value = node->value<bool>())
        {
            config.spaceAroundOperators = *value;
        }
    }

    if (auto node = table.get("proseWrap"))
    {
        if (auto value = node->value<std::string>())
        {
            config.proseWrap = *value;
        }
    }

    if (auto node = table.get("htmlWhitespaceSensitivity"))
    {
        if (auto value = node->value<std::string>())
        {
            config.htmlWhitespaceSensitivity = *value;
        }
    }

    if (auto node = table.get("vueIndentScriptAndStyle"))
    {
        if (auto value = node->value<bool>())
        {
            config.vueIndentScriptAndStyle = *value;
        }
    }

    if (auto node = table.get("embeddedLanguageFormatting"))
    {
        if (auto value = node->value<std::string>())
        {
            config.embeddedLanguageFormatting = *value;
        }
    }

    if (auto node = table.get("singleAttributePerLine"))
    {
        if (auto value = node->value<bool>())
        {
            config.singleAttributePerLine = *value;
        }
    }

    if (auto node = table.get("preserveBlankLines"))
    {
        if (auto value = node->value<bool>())
        {
            config.preserveBlankLines = *value;
        }
    }

    if (auto node = table.get("maxBlankLines"))
    {
        if (auto value = node->value<int64_t>())
        {
            int v = static_cast<int>(*value);
            if (v < 0) { v = 0; }
            if (v > 9) { v = 9; }
            config.maxBlankLines = v;
        }
    }

    if (auto node = table.get("blankLineAtFileStart"))
    {
        if (auto value = node->value<bool>())
        {
            config.blankLineAtFileStart = *value;
        }
    }

    if (auto node = table.get("blankLineAfterImports"))
    {
        if (auto value = node->value<bool>())
        {
            config.blankLineAfterImports = *value;
        }
    }

    if (auto node = table.get("blankLineBeforeReturn"))
    {
        if (auto value = node->value<bool>())
        {
            config.blankLineBeforeReturn = *value;
        }
    }

    if (auto node = table.get("blankLinesAroundTopLevelDefs"))
    {
        if (auto value = node->value<int64_t>())
        {
            int v = static_cast<int>(*value);
            if (v < 0) { v = 0; }
            if (v > 2) { v = 2; }
            config.blankLinesAroundTopLevelDefs = v;
        }
    }

    if (auto node = table.get("finalNewline"))
    {
        if (auto value = node->value<bool>())
        {
            config.finalNewline = *value;
        }
    }

    if (auto node = table.get("indentEmptyLines"))
    {
        if (auto value = node->value<bool>())
        {
            config.indentEmptyLines = *value;
        }
    }

    if (auto node = table.get("emitBom"))
    {
        if (auto value = node->value<bool>())
        {
            config.emitBom = *value;
        }
    }

    if (auto node = table.get("braceStyleFunctions"))
    {
        if (auto value = node->value<std::string>())
        {
            if (*value == "same-line" || *value == "next-line" || *value == "auto")
            {
                config.braceStyleFunctions = *value;
            }
        }
    }

    if (auto node = table.get("braceStyleTypes"))
    {
        if (auto value = node->value<std::string>())
        {
            if (*value == "same-line" || *value == "next-line" || *value == "auto")
            {
                config.braceStyleTypes = *value;
            }
        }
    }

    if (auto node = table.get("spaceAfterComma"))
    {
        if (auto value = node->value<bool>())
        {
            config.spaceAfterComma = *value;
        }
    }

    if (auto node = table.get("spaceAfterColon"))
    {
        if (auto value = node->value<bool>())
        {
            config.spaceAfterColon = *value;
        }
    }

    if (auto node = table.get("spaceBeforeFunctionParen"))
    {
        if (auto value = node->value<bool>())
        {
            config.spaceBeforeFunctionParen = *value;
        }
    }

    if (auto node = table.get("spaceInParens"))
    {
        if (auto value = node->value<bool>())
        {
            config.spaceInParens = *value;
        }
    }

    if (auto node = table.get("spaceInCondParens"))
    {
        if (auto value = node->value<bool>())
        {
            config.spaceInCondParens = *value;
        }
    }

    if (auto node = table.get("spaceInBrackets"))
    {
        if (auto value = node->value<bool>())
        {
            config.spaceInBrackets = *value;
        }
    }

    if (auto node = table.get("arrowSpacing"))
    {
        if (auto value = node->value<bool>())
        {
            config.arrowSpacing = *value;
        }
    }

    if (auto node = table.get("templateExprSpacing"))
    {
        if (auto value = node->value<bool>())
        {
            config.templateExprSpacing = *value;
        }
    }

    if (auto node = table.get("spaceAroundAssignment"))
    {
        if (auto value = node->value<bool>())
        {
            config.spaceAroundAssignment = *value;
        }
    }

    if (auto node = table.get("quoteAvoidEscape"))
    {
        if (auto value = node->value<bool>())
        {
            config.quoteAvoidEscape = *value;
        }
    }

    if (auto node = table.get("emptyBlockSameLine"))
    {
        if (auto value = node->value<bool>())
        {
            config.emptyBlockSameLine = *value;
        }
    }

    if (auto node = table.get("indentSwitchCase"))
    {
        if (auto value = node->value<bool>())
        {
            config.indentSwitchCase = *value;
        }
    }

    if (auto node = table.get("elseOnNewLine"))
    {
        if (auto value = node->value<std::string>())
        {
            if (*value == "always" || *value == "never" || *value == "auto")
            {
                config.elseOnNewLine = *value;
            }
        }
    }

    if (auto node = table.get("catchOnNewLine"))
    {
        if (auto value = node->value<std::string>())
        {
            if (*value == "always" || *value == "never" || *value == "auto")
            {
                config.catchOnNewLine = *value;
            }
        }
    }

    if (auto node = table.get("finallyOnNewLine"))
    {
        if (auto value = node->value<std::string>())
        {
            if (*value == "always" || *value == "never" || *value == "auto")
            {
                config.finallyOnNewLine = *value;
            }
        }
    }

    if (auto node = table.get("joinElseIf"))
    {
        if (auto value = node->value<bool>())
        {
            config.joinElseIf = *value;
        }
    }

    if (auto node = table.get("detectShebangLanguage"))
    {
        if (auto value = node->value<bool>())
        {
            config.detectShebangLanguage = *value;
        }
    }

    if (auto node = table.get("defaultLanguage"))
    {
        if (auto value = node->value<std::string>())
        {
            config.defaultLanguage = *value;
        }
    }

    if (auto node = table.get("language"))
    {
        if (toml::table* langs = node->as_table())
        {
            for (auto it = langs->begin(); it != langs->end(); it++)
            {
                toml::table* sub = it->second.as_table();
                if (sub == NULL) { continue; }
                std::map<std::string, std::string>& slot =
                    config.languageTextOverrides[std::string(it->first.str())];
                for (auto jt = sub->begin(); jt != sub->end(); jt++)
                {
                    std::string subKey(jt->first.str());
                    if (std::optional<std::string> s = jt->second.value<std::string>())
                    {
                        slot[subKey] = *s;
                    }
                    else if (std::optional<int64_t> n = jt->second.value<int64_t>())
                    {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "%lld", (long long)*n);
                        slot[subKey] = buf;
                    }
                    else if (std::optional<bool> b = jt->second.value<bool>())
                    {
                        slot[subKey] = *b ? "true" : "false";
                    }
                }
            }
        }
    }

    return config;
}

void ConfigBuilder::applyStringValue(Config& config, const std::string& key,
    const std::string& value)
{
    if (key == "printWidth") { config.printWidth = atoi(value.c_str()); return; }
    if (key == "tabWidth") { config.tabWidth = atoi(value.c_str()); return; }
    if (key == "maxBlankLines")
    {
        int v = atoi(value.c_str());
        if (v < 0) { v = 0; }
        if (v > 9) { v = 9; }
        config.maxBlankLines = v;
        return;
    }
    if (key == "blankLinesAroundTopLevelDefs")
    {
        int v = atoi(value.c_str());
        if (v < 0) { v = 0; }
        if (v > 2) { v = 2; }
        config.blankLinesAroundTopLevelDefs = v;
        return;
    }
    if (key == "useTabs") { config.useTabs = (value == "true"); return; }
    if (key == "semi") { config.semi = (value == "true"); return; }
    if (key == "singleQuote") { config.singleQuote = (value == "true"); return; }
    if (key == "bracketSpacing") { config.bracketSpacing = (value == "true"); return; }
    if (key == "spaceBeforeBrace") { config.spaceBeforeBrace = (value == "true"); return; }
    if (key == "spaceAfterKeyword") { config.spaceAfterKeyword = (value == "true"); return; }
    if (key == "spaceAroundOperators") { config.spaceAroundOperators = (value == "true"); return; }
    if (key == "preserveBlankLines") { config.preserveBlankLines = (value == "true"); return; }
    if (key == "blankLineAtFileStart") { config.blankLineAtFileStart = (value == "true"); return; }
    if (key == "blankLineAfterImports") { config.blankLineAfterImports = (value == "true"); return; }
    if (key == "blankLineBeforeReturn") { config.blankLineBeforeReturn = (value == "true"); return; }
    if (key == "finalNewline") { config.finalNewline = (value == "true"); return; }
    if (key == "indentEmptyLines") { config.indentEmptyLines = (value == "true"); return; }
    if (key == "emitBom") { config.emitBom = (value == "true"); return; }
    if (key == "spaceAfterComma") { config.spaceAfterComma = (value == "true"); return; }
    if (key == "spaceAfterColon") { config.spaceAfterColon = (value == "true"); return; }
    if (key == "spaceBeforeFunctionParen") { config.spaceBeforeFunctionParen = (value == "true"); return; }
    if (key == "spaceInParens") { config.spaceInParens = (value == "true"); return; }
    if (key == "spaceInCondParens") { config.spaceInCondParens = (value == "true"); return; }
    if (key == "spaceInBrackets") { config.spaceInBrackets = (value == "true"); return; }
    if (key == "arrowSpacing") { config.arrowSpacing = (value == "true"); return; }
    if (key == "templateExprSpacing") { config.templateExprSpacing = (value == "true"); return; }
    if (key == "spaceAroundAssignment") { config.spaceAroundAssignment = (value == "true"); return; }
    if (key == "quoteAvoidEscape") { config.quoteAvoidEscape = (value == "true"); return; }
    if (key == "emptyBlockSameLine") { config.emptyBlockSameLine = (value == "true"); return; }
    if (key == "indentSwitchCase") { config.indentSwitchCase = (value == "true"); return; }
    if (key == "joinElseIf") { config.joinElseIf = (value == "true"); return; }
    if (key == "detectShebangLanguage") { config.detectShebangLanguage = (value == "true"); return; }
    if (key == "trailingComma") { config.trailingComma = value; return; }
    if (key == "arrowParens") { config.arrowParens = value; return; }
    if (key == "endOfLine") { config.endOfLine = value; return; }
    if (key == "braceStyle") { config.braceStyle = value; return; }
    if (key == "braceStyleFunctions")
    {
        if (value == "same-line" || value == "next-line" || value == "auto") { config.braceStyleFunctions = value; }
        return;
    }
    if (key == "braceStyleTypes")
    {
        if (value == "same-line" || value == "next-line" || value == "auto") { config.braceStyleTypes = value; }
        return;
    }
    if (key == "elseOnNewLine")
    {
        if (value == "always" || value == "never" || value == "auto") { config.elseOnNewLine = value; }
        return;
    }
    if (key == "catchOnNewLine")
    {
        if (value == "always" || value == "never" || value == "auto") { config.catchOnNewLine = value; }
        return;
    }
    if (key == "finallyOnNewLine")
    {
        if (value == "always" || value == "never" || value == "auto") { config.finallyOnNewLine = value; }
        return;
    }
    if (key == "proseWrap") { config.proseWrap = value; return; }
    if (key == "htmlWhitespaceSensitivity") { config.htmlWhitespaceSensitivity = value; return; }
    if (key == "embeddedLanguageFormatting") { config.embeddedLanguageFormatting = value; return; }
    if (key == "defaultLanguage") { config.defaultLanguage = value; return; }
    if (key == "vueIndentScriptAndStyle") { config.vueIndentScriptAndStyle = (value == "true"); return; }
    if (key == "singleAttributePerLine") { config.singleAttributePerLine = (value == "true"); return; }
}

void ConfigBuilder::applyLanguageOverrides(Config& config, const std::string& language)
{
    std::map<std::string, std::map<std::string, std::string> >::const_iterator it =
        config.languageTextOverrides.find(language);
    if (it == config.languageTextOverrides.end()) { return; }
    for (std::map<std::string, std::string>::const_iterator jt = it->second.begin();
        jt != it->second.end(); jt++)
    {
        applyStringValue(config, jt->first, jt->second);
    }
}

Config ConfigBuilder::buildDefault()
{
    // Easter egg: building a default config, fresh from the factory
    return Config();
}

Config ConfigBuilder::buildForFile(const std::string& filepath)
{
    // ponytail: fopen probe + string walk; no <filesystem> so this stays C++11
    std::string dir;
    size_t slash = filepath.find_last_of("/\\");
    dir = (slash == std::string::npos) ? std::string(".") : filepath.substr(0, slash);
    if (dir.empty())
    {
        dir = "/";
    }

    for (;;)
    {
        std::string candidate = dir + "/.fmtpp.toml";
        FILE* probe = fopen(candidate.c_str(), "r");
        if (probe != NULL)
        {
            fclose(probe);
            return buildFromFile(candidate);
        }
        if (dir == "." || dir == "/" || dir == "\\" ||
            (dir.size() == 2 && dir[1] == ':'))
        {
            break;
        }
        size_t parent = dir.find_last_of("/\\");
        if (parent == std::string::npos)
        {
            dir = ".";
        }
        else if (parent == 0)
        {
            dir = "/";
        }
        else
        {
            dir = dir.substr(0, parent);
        }
    }

    return buildDefault();
}
