#include "fmtpp/FormatterApp.hpp"
#include "fmtpp/EasterEggs.hpp"

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
        // Print usage information
        printf("Usage: %s [OPTIONS]\n", m_programName.c_str());
        printf("Options:\n");
        printf("  --version      Print version information\n");
        printf("  --easter-egg   Print a random easter egg\n");
        goto cleanup;
    }

    // Easter egg: manual argument parsing without CLI11
    if (std::strcmp(argv[1], "--version") == 0)
    {
        printVersion();
    }
    else if (std::strcmp(argv[1], "--easter-egg") == 0)
    {
        printEasterEgg();
    }
    else
    {
        printf("Unknown option: %s\n", argv[1]);
        printf("Usage: %s [OPTIONS]\n", m_programName.c_str());
        printf("Options:\n");
        printf("  --version      Print version information\n");
        printf("  --easter-egg   Print a random easter egg\n");
        result = 1;
    }

cleanup:
    return result;
}

void FormatterApp::printVersion()
{
    printf("fmtpp 0.1.0\n");
}

void FormatterApp::printEasterEgg()
{
    // Easter egg: seed the random number generator with the current time
    std::srand(static_cast<unsigned int>(std::time(NULL)));
    int index = std::rand() % g_easterEggCount;
    printf("%s\n", g_easterEggs[index]);
}
