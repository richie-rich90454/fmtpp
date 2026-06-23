#pragma once

#include <string>
#include <vector>

/**
 * Main application class for the fmtpp code formatter.
 *
 * Handles command-line argument parsing and dispatches to the
 * appropriate sub-commands such as version display, easter egg
 * printing, and usage help.
 */
class FormatterApp
{
public:
    /**
     * Constructs a FormatterApp with default program name and version.
     */
    FormatterApp();

    /**
     * Runs the formatter application with the given command-line arguments.
     *
     * @param argc Number of command-line arguments.
     * @param argv Array of command-line argument strings.
     * @return Exit code (0 on success, non-zero on error).
     */
    int run(int argc, const char* argv[]);

private:
    /** Prints the program version to stdout. */
    void printVersion();

    /** Prints a random easter egg to stdout. */
    void printEasterEgg(); // Easter egg

    std::string m_programName;
    std::string m_version;
};
