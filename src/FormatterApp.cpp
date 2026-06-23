#include "fmtpp/FormatterApp.hpp"
#include "fmtpp/EasterEggs.hpp"
#include "fmtpp/TuiConfigurator.hpp"
#include "fmtpp/StyleDetector.hpp"
#include "fmtpp/ThreadPool.hpp"
#include "fmtpp/SourceBuffer.hpp"
#include "fmtpp/Parser.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/JSFormatter.hpp"
#include "fmtpp/TSFormatter.hpp"
#include "fmtpp/JavaFormatter.hpp"
#include "fmtpp/CFormatter.hpp"
#include "fmtpp/CppFormatter.hpp"
#include "fmtpp/CSharpFormatter.hpp"
#include "fmtpp/GoFormatter.hpp"
#include "fmtpp/RustFormatter.hpp"

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>

FormatterApp::FormatterApp()
    : m_programName("fmtpp")
    , m_version("0.1.0")
{
}

int FormatterApp::run(int argc, const char* argv[])
{
    int result = 0;

    if (argc < 2)
    {
        printUsage();
        goto cleanup;
    }

    // Easter egg: manual argument parsing - old school style
    if (std::strcmp(argv[1], "--version") == 0 || std::strcmp(argv[1], "-v") == 0)
    {
        printVersion();
    }
    else if (std::strcmp(argv[1], "--easter-egg") == 0)
    {
        printEasterEgg();
    }
    else if (std::strcmp(argv[1], "--config") == 0 || std::strcmp(argv[1], "config") == 0)
    {
        result = runConfigurator();
    }
    else if (std::strcmp(argv[1], "--learn") == 0 || std::strcmp(argv[1], "learn") == 0)
    {
        result = runLearn(argc, argv);
    }
    else if (std::strcmp(argv[1], "--format") == 0 || std::strcmp(argv[1], "format") == 0)
    {
        result = runFormat(argc, argv);
    }
    else if (std::strcmp(argv[1], "--help") == 0 || std::strcmp(argv[1], "-h") == 0)
    {
        printUsage();
    }
    else
    {
        // Assume it's a file to format
        result = runFormatSingle(argv[1]);
    }

cleanup:
    return result;
}

void FormatterApp::printUsage()
{
    printf("fmtpp v%s - A professional code formatter\n", m_version.c_str());
    printf("Usage: %s [OPTIONS] [FILE...]\n\n", m_programName.c_str());
    printf("Options:\n");
    printf("  --version, -v     Print version information\n");
    printf("  --help, -h        Print this help message\n");
    printf("  --easter-egg      Print a random easter egg\n");
    printf("  --config, config  Launch interactive TUI configurator\n");
    printf("  --learn [FILE]    Learn style from a file and generate .fmtpp.toml\n");
    printf("  --format FILE...  Format one or more files\n");
    printf("  FILE              Format a single file\n");
    printf("\nSupported languages: js, ts, java, c, cpp, cs, go, rust\n");
    printf("\nEaster egg: fmtpp - formatting code with style and zero leaks\n");
}

void FormatterApp::printVersion()
{
    printf("fmtpp %s\n", m_version.c_str());
    printf("Easter egg: version %s - the one that started it all\n", m_version.c_str());
}

void FormatterApp::printEasterEgg()
{
    // Easter egg: seed the random number generator with the current time
    std::srand(static_cast<unsigned int>(std::time(NULL)));
    int index = std::rand() % g_easterEggCount;
    printf("%s\n", g_easterEggs[index]);
}

int FormatterApp::runConfigurator()
{
    Config config;
    TuiConfigurator tui;
    bool saved = tui.run(config);

    if (saved)
    {
        printf("Config saved to .fmtpp.toml\n");
        printf("Easter egg: your formatting preferences have been recorded for posterity\n");
    }
    else
    {
        printf("Config discarded.\n");
    }

    return 0;
}

int FormatterApp::runLearn(int argc, const char* argv[])
{
    const char* filepath = ".fmtpp.toml";
    const char* sourceFile = NULL;

    if (argc >= 3)
    {
        sourceFile = argv[2];
    }
    else
    {
        printf("Usage: %s --learn <file>\n", m_programName.c_str());
        return 1;
    }

    if (argc >= 4)
    {
        filepath = argv[3];
    }

    SourceBuffer source(sourceFile);
    if (!source.isValid())
    {
        printf("Error: Cannot read file: %s\n", sourceFile);
        return 1;
    }

    Parser parser;
    ProgramNode* ast = parser.parse(source);
    if (ast == NULL)
    {
        printf("Error: Cannot parse file: %s\n", sourceFile);
        return 1;
    }

    StyleDetector detector;
    Config detected = detector.detect(ast);

    if (detector.writeConfig(filepath))
    {
        printf("Style detected and written to %s\n", filepath);
        printf("Detected settings:\n");
        printf("  printWidth: %d\n", detected.printWidth);
        printf("  useTabs: %s\n", detected.useTabs ? "true" : "false");
        printf("  semi: %s\n", detected.semi ? "true" : "false");
        printf("  singleQuote: %s\n", detected.singleQuote ? "true" : "false");
        printf("  braceStyle: %s\n", detected.braceStyle.c_str());
        printf("Easter egg: the style detective has learned your secrets\n");
    }
    else
    {
        printf("Error: Cannot write config file: %s\n", filepath);
        return 1;
    }

    return 0;
}

int FormatterApp::runFormat(int argc, const char* argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s --format <file...>\n", m_programName.c_str());
        return 1;
    }

    // Detect language from file extension
    std::string language = detectLanguage(argv[2]);

    if (argc == 3)
    {
        // Single file
        return runFormatSingle(argv[2]);
    }

    // Multiple files - use thread pool
    std::vector<std::string> files;
    int i;
    for (i = 2; i < argc; i++)
    {
        files.push_back(argv[i]);
    }

    ThreadPool pool(0); // Use hardware concurrency
    printf("Formatting %d files with %zu threads...\n",
        static_cast<int>(files.size()), pool.getWorkerCount());

    std::vector<std::string> results = pool.parallelFormat(files, language);

    int successCount = 0;
    int failCount = 0;
    for (i = 0; i < static_cast<int>(results.size()); i++)
    {
        if (!results[i].empty())
        {
            printf("  Formatted: %s\n", files[i].c_str());
            successCount++;
        }
        else
        {
            printf("  Failed: %s\n", files[i].c_str());
            failCount++;
        }
    }

    printf("Done: %d succeeded, %d failed\n", successCount, failCount);
    printf("Easter egg: the thread pool has done its parallel dance\n");

    return (failCount > 0) ? 1 : 0;
}

int FormatterApp::runFormatSingle(const char* filepath)
{
    SourceBuffer source(filepath);
    if (!source.isValid())
    {
        printf("Error: Cannot read file: %s\n", filepath);
        return 1;
    }

    Parser parser;
    ProgramNode* ast = parser.parse(source);
    if (ast == NULL)
    {
        printf("Error: Cannot parse file: %s\n", filepath);
        return 1;
    }

    std::string language = detectLanguage(filepath);
    Config config;
    OutputBuffer out(static_cast<size_t>(source.getSize()) * 2);
    FormatterVisitor* formatter = NULL;

    // Select formatter based on language
    if (language == "js" || language == "javascript")
    {
        formatter = new JSFormatter(out, config);
    }
    else if (language == "ts" || language == "typescript")
    {
        formatter = new TSFormatter(out, config);
    }
    else if (language == "java")
    {
        formatter = new JavaFormatter(out, config);
    }
    else if (language == "c")
    {
        formatter = new CFormatter(out, config);
    }
    else if (language == "cpp" || language == "c++")
    {
        formatter = new CppFormatter(out, config);
    }
    else if (language == "cs" || language == "csharp" || language == "c#")
    {
        formatter = new CSharpFormatter(out, config);
    }
    else if (language == "go")
    {
        formatter = new GoFormatter(out, config);
    }
    else if (language == "rs" || language == "rust")
    {
        formatter = new RustFormatter(out, config);
    }
    else
    {
        // Default to JavaScript
        formatter = new JSFormatter(out, config);
    }

    if (formatter != NULL)
    {
        formatter->format(ast);
        printf("%s", out.toString().c_str());
        delete formatter;
        return 0;
    }

    return 1;
}

std::string FormatterApp::detectLanguage(const char* filepath)
{
    if (filepath == NULL)
    {
        return "js";
    }

    // Find the last dot in the filename
    const char* dot = strrchr(filepath, '.');
    if (dot == NULL)
    {
        return "js";
    }

    const char* ext = dot + 1;

    if (strcmp(ext, "js") == 0 || strcmp(ext, "mjs") == 0 || strcmp(ext, "cjs") == 0)
    {
        return "js";
    }
    if (strcmp(ext, "ts") == 0 || strcmp(ext, "tsx") == 0 || strcmp(ext, "mts") == 0)
    {
        return "ts";
    }
    if (strcmp(ext, "java") == 0)
    {
        return "java";
    }
    if (strcmp(ext, "c") == 0 || strcmp(ext, "h") == 0)
    {
        return "c";
    }
    if (strcmp(ext, "cpp") == 0 || strcmp(ext, "cc") == 0 || strcmp(ext, "cxx") == 0 ||
        strcmp(ext, "hpp") == 0 || strcmp(ext, "hh") == 0 || strcmp(ext, "hxx") == 0)
    {
        return "cpp";
    }
    if (strcmp(ext, "cs") == 0)
    {
        return "cs";
    }
    if (strcmp(ext, "go") == 0)
    {
        return "go";
    }
    if (strcmp(ext, "rs") == 0)
    {
        return "rs";
    }

    return "js";
}