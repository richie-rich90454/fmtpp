#include "fmtpp/ConfigBuilder.hpp"
#include <toml++/toml.hpp>
#include <cstdio>

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

    return config;
}

Config ConfigBuilder::buildDefault()
{
    // Easter egg: building a default config, fresh from the factory
    return Config();
}
