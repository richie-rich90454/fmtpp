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

    // Easter egg: building configs, one TOML at a time
};