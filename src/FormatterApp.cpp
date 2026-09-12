#include "fmtpp/FormatterApp.hpp"
#include "fmtpp/EasterEggs.hpp"
#include "fmtpp/TuiConfigurator.hpp"
#include "fmtpp/StyleDetector.hpp"
#include "fmtpp/ThreadPool.hpp"
#include "fmtpp/Benchmark.hpp"
#include "fmtpp/SourceBuffer.hpp"
#include "fmtpp/Parser.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/ConfigBuilder.hpp"
#include "fmtpp/FormatterFactory.hpp"
#include "fmtpp/FormatterVisitor.hpp"
#include "CLI/CLI.hpp"

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
    CLI::App app("fmtpp v" + m_version + " - A professional code formatter", m_programName);
    app.set_version_flag("--version,-v", std::string("fmtpp ") + m_version);
    app.set_help_flag("--help,-h", "Print this help message");

    bool easterEgg = false;
    bool cowbell = false;
    bool kittens = false;
    bool unicorn = false;

    app.add_flag("--easter-egg", easterEgg, "Print a random easter egg")->group("");
    app.add_flag("--cowbell", cowbell, "More cowbell!")->group("");
    app.add_flag("--kittens", kittens, "Print ASCII kitten art")->group("");
    app.add_flag("--unicorn", unicorn, "Print ASCII unicorn art")->group("");

    std::vector<std::string> directFiles;
    app.add_option("files", directFiles, "Format a single file directly");

    CLI::App* formatCmd = app.add_subcommand("format", "Format one or more files");
    std::vector<std::string> formatFiles;
    formatCmd->add_option("files", formatFiles, "Files to format")->required();

    CLI::App* benchCmd = app.add_subcommand("bench", "Run performance benchmark");
    std::vector<std::string> benchFiles;
    int benchRuns = 10;
    std::string compareTool;
    benchCmd->add_option("files", benchFiles, "Files to benchmark")->required();
    benchCmd->add_option("--runs", benchRuns, "Number of iterations");
    benchCmd->add_option("--compare", compareTool, "External formatter to compare (oxfmt, prettier)");

    CLI::App* configCmd = app.add_subcommand("config", "Launch interactive TUI configurator");
    (void)configCmd;

    CLI::App* learnCmd = app.add_subcommand("learn", "Learn style from a file and generate .fmtpp.toml");
    std::string learnFile;
    std::string learnOutput = ".fmtpp.toml";
    learnCmd->add_option("file", learnFile, "Source file to learn from")->required();
    learnCmd->add_option("--output", learnOutput, "Output config file path");

    try
    {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError& e)
    {
        return app.exit(e);
    }

    if (easterEgg)
    {
        printEasterEgg();
        return 0;
    }
    if (cowbell)
    {
        printCowbell();
        return 0;
    }
    if (kittens)
    {
        printKittens();
        return 0;
    }
    if (unicorn)
    {
        printUnicorn();
        return 0;
    }

    if (app.got_subcommand("format"))
    {
        return runFormat(formatFiles);
    }
    if (app.got_subcommand("bench"))
    {
        return runBenchmark(benchFiles, benchRuns, compareTool);
    }
    if (app.got_subcommand("config"))
    {
        return runConfigurator();
    }
    if (app.got_subcommand("learn"))
    {
        return runLearn(learnFile, learnOutput);
    }

    if (!directFiles.empty())
    {
        return runFormatSingle(directFiles[0]);
    }

    printUsage();
    return 0;
}

void FormatterApp::printUsage()
{
    printf("fmtpp v%s - A professional code formatter\n", m_version.c_str());
    printf("Usage: %s [OPTIONS] [FILE...]\n\n", m_programName.c_str());
    printf("Options:\n");
    printf("  --version, -v     Print version information\n");
    printf("  --help, -h        Print this help message\n");
    printf("  --easter-egg      Print a random easter egg\n");
    printf("  config            Launch interactive TUI configurator\n");
    printf("  learn [FILE]      Learn style from a file and generate .fmtpp.toml\n");
    printf("  format FILE...    Format one or more files\n");
    printf("  bench FILE        Run performance benchmark\n");
    printf("  FILE              Format a single file\n");
    printf("\nSupported languages: js, ts, java, c, cpp, cs, go, rs,\n");
    printf("  kotlin, swift, dart, php, scala, objc, zig, d, groovy,\n");
    printf("  solidity, v, glsl, html, css, json, vue, python, ruby\n");
    printf("\nfmtpp - formatting code with style and zero leaks\n");
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

void FormatterApp::printCowbell()
{
    // Easter egg: the legendary cowbell, hidden in plain sight
    printf("More cowbell!\n");
}

void FormatterApp::printKittens()
{
    // Easter egg: ASCII art kittens for the brave formatter explorer
    printf(" /\\_/\\\n");
    printf("( o.o )\n");
    printf(" > ^ <\n");
    printf("Meow! Kittens approve of your formatting.\n");
}

void FormatterApp::printUnicorn()
{
    // Easter egg: a majestic ASCII unicorn guarding the formatter
    printf("    \\   /\\\n");
    printf("     )  ( ')\n");
    printf("    (  /  )\n");
    printf("     \\(__)|\n");
    printf("Neigh! The formatting unicorn has blessed your code.\n");
}

int FormatterApp::runConfigurator()
{
    Config config;
    TuiConfigurator tui;
    bool saved = tui.run(config);

    if (saved)
    {
        printf("Config saved to .fmtpp.toml\n");
        printf("Your formatting preferences have been recorded for posterity\n");
    }
    else
    {
        printf("Config discarded.\n");
    }

    return 0;
}

int FormatterApp::runLearn(const std::string& sourceFile, const std::string& outputFile)
{
    SourceBuffer source(sourceFile.c_str());
    if (!source.isValid())
    {
        printf("Error: Cannot read file: %s\n", sourceFile.c_str());
        return 1;
    }

    Parser parser;
    ProgramNode* ast = parser.parse(source);
    if (ast == NULL)
    {
        printf("Error: Cannot parse file: %s\n", sourceFile.c_str());
        return 1;
    }

    StyleDetector detector;
    // ponytail: detect from raw bytes; the stub parser cannot see style
    Config detected = detector.detectFromSource(source.getData(), source.getSize());

    if (detector.writeConfig(outputFile.c_str()))
    {
        printf("Style detected and written to %s\n", outputFile.c_str());
        printf("Detected settings:\n");
        printf("  printWidth: %d\n", detected.printWidth);
        printf("  tabWidth: %d\n", detected.tabWidth);
        printf("  useTabs: %s\n", detected.useTabs ? "true" : "false");
        printf("  semi: %s\n", detected.semi ? "true" : "false");
        printf("  singleQuote: %s\n", detected.singleQuote ? "true" : "false");
        printf("  braceStyle: %s\n", detected.braceStyle.c_str());
        printf("  endOfLine: %s\n", detected.endOfLine.c_str());
        printf("The style detective has learned your secrets\n");
    }
    else
    {
        printf("Error: Cannot write config file: %s\n", outputFile.c_str());
        return 1;
    }

    return 0;
}

int FormatterApp::runFormat(const std::vector<std::string>& files)
{
    if (files.empty())
    {
        printf("Usage: %s format <file...>\n", m_programName.c_str());
        return 1;
    }

    std::string language = detectLanguage(files[0].c_str());

    if (files.size() == 1)
    {
        return runFormatSingle(files[0]);
    }

    ThreadPool pool(0);
    printf("Formatting %zu files with %zu threads...\n",
        files.size(), pool.getWorkerCount());

    std::vector<std::string> results = pool.parallelFormat(files, language);

    size_t successCount = 0;
    size_t failCount = 0;
    for (size_t i = 0; i < results.size(); i++)
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

    printf("Done: %zu succeeded, %zu failed\n", successCount, failCount);
    printf("The thread pool has done its parallel dance\n");

    return (failCount > 0) ? 1 : 0;
}

int FormatterApp::runBenchmark(const std::vector<std::string>& files, int runs, const std::string& compareTool)
{
    if (files.empty())
    {
        printf("Usage: %s bench <file> [--runs N] [--compare <oxfmt|prettier>]\n", m_programName.c_str());
        return 1;
    }

    Benchmark::Config benchConfig;
    benchConfig.iterations = runs;
    benchConfig.warmup = true;
    benchConfig.verbose = true;
    benchConfig.language = detectLanguage(files[0].c_str());
    benchConfig.files = files;
    benchConfig.compareTool = compareTool;

    printf("fmtpp Benchmark\n");
    printf("  Files: %zu\n", benchConfig.files.size());
    printf("  Iterations: %d\n", benchConfig.iterations);
    printf("  Language: %s\n", benchConfig.language.c_str());
    printf("  Warmup: %s\n", benchConfig.warmup ? "yes" : "no");
    if (!benchConfig.compareTool.empty())
    {
        printf("  Compare tool: %s\n", benchConfig.compareTool.c_str());
    }
    printf("The benchmarker is warming up...\n");

    Benchmark bench;
    Benchmark::Result result = bench.run(benchConfig);
    bench.printReport(result);

    if (!benchConfig.compareTool.empty())
    {
        Benchmark::Result baseline = bench.runCompare(benchConfig, result);
        if (baseline.filesProcessed > 0)
        {
            bench.printCompareReport(result, baseline, benchConfig.compareTool);
        }
        else
        {
            printf("Warning: Could not run comparison tool '%s'. Is it installed?\n",
                benchConfig.compareTool.c_str());
            printf("The comparison tool is playing hide and seek\n");
        }
    }

    return 0;
}

int FormatterApp::runFormatSingle(const std::string& filepath)
{
    SourceBuffer source(filepath.c_str());
    if (!source.isValid())
    {
        printf("Error: Cannot read file: %s\n", filepath.c_str());
        return 1;
    }

    Parser parser;
    std::string language = FormatterFactory::detectLanguage(filepath.c_str());
    ProgramNode* ast = parser.parse(source, language);
    if (ast == NULL)
    {
        printf("Error: Cannot parse file: %s\n", filepath.c_str());
        return 1;
    }

    // ponytail: .fmtpp.toml next to the file wins; defaults when missing or broken
    Config config = ConfigBuilder::buildForFile(filepath);
    OutputBuffer out(static_cast<size_t>(source.getSize()) * 2);
    FormatterVisitor* formatter = FormatterFactory::create(language, out, config);

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
    return FormatterFactory::detectLanguage(filepath);
}
