#pragma once
#include "Config.hpp"
#include <string>

/**
 * Builds a Config from various sources.
 * Reads .fmtpp.toml files using the toml++ library and merges with defaults.
 */
class ConfigBuilder
{
public:
    /**
     * Builds a Config from a .fmtpp.toml file.
     * Merges the file settings with defaults.
     * @param filepath Path to the TOML config file.
     * @return Config with merged settings.
     */
    static Config buildFromFile(const std::string& filepath);

    /**
     * Builds a Config with default Prettier-matching values.
     * @return Default Config.
     */
    static Config buildDefault();

    /**
     * Builds a Config for a given source file.
     * Looks for .fmtpp.toml in the file's directory, then each parent
     * directory; returns defaults when no config file is found.
     * Missing or broken files also yield defaults.
     * @param filepath Path of the source file being formatted.
     * @return Config with merged settings.
     */
    static Config buildForFile(const std::string& filepath);

    // Easter egg: building configs, one TOML at a time
};