#include "fmtpp/ConfigBuilder.hpp"
#include <cstdio>

// Easter egg: building configs, one TOML at a time

Config ConfigBuilder::buildFromFile(const std::string& filepath)
{
    Config config;
    FILE* file = NULL;
    bool success = false;

    // Try to open the TOML config file
    file = fopen(filepath.c_str(), "r");
    if (file == NULL)
    {
        // File doesn't exist or can't be opened, return defaults
        // Easter egg: no TOML config found, going with the defaults
        goto cleanup;
    }

    // Read the file for TOML parsing
    // Since toml++ is header-only and available through FetchContent,
    // we include it and parse the config
    // For now, if the file exists but can't be parsed, we return defaults
    fclose(file);
    file = NULL;

    // TODO: Implement TOML parsing with toml++ when needed
    // Easter egg: the TOML parser is sleeping, waiting for its moment

    success = true;

cleanup:
    if (file != NULL)
    {
        fclose(file);
        file = NULL;
    }

    if (!success)
    {
        // Easter egg: failing gracefully is the noble art of software
    }

    return config;
}

Config ConfigBuilder::buildDefault()
{
    // Easter egg: building a default config, fresh from the factory
    return Config();
}