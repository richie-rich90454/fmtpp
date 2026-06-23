#include "fmtpp/Benchmark.hpp"
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
#include "fmtpp/MemoryPool.hpp"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

// Easter egg: the benchmarker - where milliseconds become bragging rights

Benchmark::Benchmark()
{
}

Benchmark::~Benchmark()
{
}

double Benchmark::getTimeMs()
{
#ifdef _WIN32
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (counter.QuadPart * 1000.0) / freq.QuadPart;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
#endif
}

double Benchmark::formatFile(const std::string& filepath, const std::string& language)
{
    double startTime = getTimeMs();

    SourceBuffer source(filepath.c_str());
    if (!source.isValid())
    {
        return -1.0;
    }

    Parser parser;
    ProgramNode* ast = parser.parse(source);
    if (ast == NULL)
    {
        return -1.0;
    }

    ::Config fmtConfig;
    OutputBuffer out(static_cast<size_t>(source.getSize()) * 2);
    FormatterVisitor* formatter = NULL;

    if (language == "js" || language == "javascript")
    {
        formatter = new JSFormatter(out, fmtConfig);
    }
    else if (language == "ts" || language == "typescript")
    {
        formatter = new TSFormatter(out, fmtConfig);
    }
    else if (language == "java")
    {
        formatter = new JavaFormatter(out, fmtConfig);
    }
    else if (language == "c")
    {
        formatter = new CFormatter(out, fmtConfig);
    }
    else if (language == "cpp" || language == "c++")
    {
        formatter = new CppFormatter(out, fmtConfig);
    }
    else if (language == "cs" || language == "csharp" || language == "c#")
    {
        formatter = new CSharpFormatter(out, fmtConfig);
    }
    else if (language == "go")
    {
        formatter = new GoFormatter(out, fmtConfig);
    }
    else if (language == "rs" || language == "rust")
    {
        formatter = new RustFormatter(out, fmtConfig);
    }
    else
    {
        formatter = new JSFormatter(out, fmtConfig);
    }

    double elapsed = -1.0;
    if (formatter != NULL)
    {
        formatter->format(ast);
        std::string formatted = out.toString();
        delete formatter;
        elapsed = getTimeMs() - startTime;
    }

    return elapsed;
}

Benchmark::Result Benchmark::run(const Config& config)
{
    Result result;
    result.totalTimeMs = 0.0;
    result.averageTimeMs = 0.0;
    result.minTimeMs = 1e9;
    result.maxTimeMs = 0.0;
    result.filesProcessed = 0;
    result.totalBytesProcessed = 0;
    result.throughputBytesPerSec = 0;
    result.memoryPoolUsed = 0;
    result.memoryPoolCapacity = 0;

    if (config.files.empty())
    {
        return result;
    }

    // Warmup run
    if (config.warmup && !config.files.empty())
    {
        if (config.verbose)
        {
            printf("  Warmup run...\n");
        }
        formatFile(config.files[0], config.language);
    }

    // Benchmark runs
    int iter;
    for (iter = 0; iter < config.iterations; iter++)
    {
        if (config.verbose)
        {
            printf("  Iteration %d/%d...\n", iter + 1, config.iterations);
        }

        size_t i;
        for (i = 0; i < config.files.size(); i++)
        {
            double fileTime = formatFile(config.files[i], config.language);

            if (fileTime >= 0.0)
            {
                result.totalTimeMs += fileTime;
                result.filesProcessed++;

                if (fileTime < result.minTimeMs)
                {
                    result.minTimeMs = fileTime;
                }
                if (fileTime > result.maxTimeMs)
                {
                    result.maxTimeMs = fileTime;
                }

                // Get file size
                SourceBuffer sb(config.files[i].c_str());
                if (sb.isValid())
                {
                    result.totalBytesProcessed += sb.getSize();
                }
            }
        }
    }

    // Calculate averages
    if (result.filesProcessed > 0)
    {
        result.averageTimeMs = result.totalTimeMs / static_cast<double>(result.filesProcessed);
    }

    // Calculate throughput
    if (result.totalTimeMs > 0.0)
    {
        double totalSeconds = result.totalTimeMs / 1000.0;
        result.throughputBytesPerSec = static_cast<size_t>(
            static_cast<double>(result.totalBytesProcessed) / totalSeconds);
    }

    // Memory pool stats
    if (g_pool != NULL)
    {
        result.memoryPoolUsed = g_pool->getUsed();
        result.memoryPoolCapacity = g_pool->getCapacity();
    }

    return result;
}

void Benchmark::printReport(const Result& result)
{
    printf("\n");
    printf("  ========================================\n");
    printf("  fmtpp Benchmark Report\n");
    printf("  ========================================\n");
    printf("  Files processed:     %zu\n", result.filesProcessed);
    printf("  Total bytes:         %zu\n", result.totalBytesProcessed);
    printf("  Total time:          %.2f ms\n", result.totalTimeMs);
    printf("  Average per file:    %.2f ms\n", result.averageTimeMs);
    printf("  Min time:            %.2f ms\n", result.minTimeMs);
    printf("  Max time:            %.2f ms\n", result.maxTimeMs);
    printf("  Throughput:          %zu bytes/sec\n", result.throughputBytesPerSec);

    if (result.throughputBytesPerSec > 1024 * 1024)
    {
        printf("  Throughput:          %.2f MB/sec\n",
            static_cast<double>(result.throughputBytesPerSec) / (1024.0 * 1024.0));
    }

    printf("  Memory pool used:    %zu / %zu bytes\n",
        result.memoryPoolUsed, result.memoryPoolCapacity);

    printf("  ========================================\n");

    // Performance verdict
    if (result.averageTimeMs < 12.0)
    {
        printf("  VERDICT: Beats Oxfmt (<12ms)! \n");
        printf("  Easter egg: faster than a speeding bullet train\n");
    }
    else if (result.averageTimeMs < 50.0)
    {
        printf("  VERDICT: Good performance\n");
        printf("  Easter egg: not quite lightning, but pretty fast\n");
    }
    else
    {
        printf("  VERDICT: Needs optimization\n");
        printf("  Easter egg: even turtles can win races with determination\n");
    }

    printf("  ========================================\n\n");
}