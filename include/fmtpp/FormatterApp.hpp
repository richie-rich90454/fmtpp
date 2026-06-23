#pragma once

#include <string>
#include <vector>

/**
 * Main application class for the fmtpp code formatter.
 *
 * Handles command-line argument parsing and dispatches to the
 * appropriate sub-commands such as version display, easter egg
 * printing, TUI configuration, style learning, and file formatting.
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
    /** Prints usage information to stdout. */
    void printUsage();

    /** Prints the program version to stdout. */
    void printVersion();

    /** Prints a random easter egg to stdout. */
    void printEasterEgg(); // Easter egg

    /** Launches the interactive TUI configurator. */
    int runConfigurator();

    /** Runs the style learning command. */
    int runLearn(int argc, const char* argv[]);

    /** Formats one or more files. */
    int runFormat(int argc, const char* argv[]);

    /** Runs a performance benchmark. */
    int runBenchmark(int argc, const char* argv[]);

    /** Formats a single file and prints to stdout. */
    int runFormatSingle(const char* filepath);

    /**
     * Detects the language from a file extension.
     * @param filepath The file path to analyze.
     * @return Language identifier string (js, ts, java, c, cpp, cs, go, rs).
     */
    std::string detectLanguage(const char* filepath);

    std::string m_programName;
    std::string m_version;
};