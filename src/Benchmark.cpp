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
#include <vector>

#ifdef _WIN32
#include <windows.h>
#define popen _popen
#define pclose _pclose
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

double Benchmark::computePercentile(const std::vector<double>& samples, double percentile) const
{
    if (samples.empty())
    {
        return 0.0;
    }

    size_t n = samples.size();
    if (n == 1)
    {
        return samples[0];
    }

    double position = percentile * static_cast<double>(n - 1);
    size_t lower = static_cast<size_t>(position);
    size_t upper = lower + 1;
    if (upper >= n)
    {
        return samples[n - 1];
    }

    double weight = position - static_cast<double>(lower);
    return samples[lower] * (1.0 - weight) + samples[upper] * weight;
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

double Benchmark::formatFileExternal(const std::string& filepath, const std::string& tool)
{
    double startTime = getTimeMs();

    std::string command;
    if (tool == "prettier")
    {
        command = "prettier ";
    }
    else if (tool == "oxfmt")
    {
        command = "oxfmt ";
    }
    else
    {
        command = tool + " ";
    }
    command += filepath;

    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == NULL)
    {
        return -1.0;
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        // Drain output to ensure the tool actually ran
    }

    int status = pclose(pipe);
    if (status != 0)
    {
        return -1.0;
    }

    return getTimeMs() - startTime;
}

Benchmark::Result Benchmark::run(const Config& config)
{
    Result result;
    std::vector<double> samples;

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
                samples.push_back(fileTime);
                result.totalTimeMs += fileTime;
                result.filesProcessed++;

                if (result.filesProcessed == 1 || fileTime < result.minTimeMs)
                {
                    result.minTimeMs = fileTime;
                }
                if (result.filesProcessed == 1 || fileTime > result.maxTimeMs)
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

    // Calculate statistics
    if (result.filesProcessed > 0)
    {
        result.averageTimeMs = result.totalTimeMs / static_cast<double>(result.filesProcessed);
    }

    if (!samples.empty())
    {
        std::sort(samples.begin(), samples.end());
        result.medianTimeMs = computePercentile(samples, 0.5);
        result.p95TimeMs = computePercentile(samples, 0.95);
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

Benchmark::Result Benchmark::runCompare(const Config& config, const Result& fmtppResult)
{
    Result baseline;
    std::vector<double> samples;

    if (config.files.empty() || config.compareTool.empty())
    {
        return baseline;
    }

    // Warmup run for external tool
    if (config.warmup && !config.files.empty())
    {
        if (config.verbose)
        {
            printf("  Warmup run (%s)...\n", config.compareTool.c_str());
        }
        formatFileExternal(config.files[0], config.compareTool);
    }

    // Benchmark runs for external tool
    int iter;
    for (iter = 0; iter < config.iterations; iter++)
    {
        if (config.verbose)
        {
            printf("  Iteration %d/%d (%s)...\n", iter + 1, config.iterations, config.compareTool.c_str());
        }

        size_t i;
        for (i = 0; i < config.files.size(); i++)
        {
            double fileTime = formatFileExternal(config.files[i], config.compareTool);

            if (fileTime >= 0.0)
            {
                samples.push_back(fileTime);
                baseline.totalTimeMs += fileTime;
                baseline.filesProcessed++;

                if (baseline.filesProcessed == 1 || fileTime < baseline.minTimeMs)
                {
                    baseline.minTimeMs = fileTime;
                }
                if (baseline.filesProcessed == 1 || fileTime > baseline.maxTimeMs)
                {
                    baseline.maxTimeMs = fileTime;
                }

                SourceBuffer sb(config.files[i].c_str());
                if (sb.isValid())
                {
                    baseline.totalBytesProcessed += sb.getSize();
                }
            }
        }
    }

    if (baseline.filesProcessed > 0)
    {
        baseline.averageTimeMs = baseline.totalTimeMs / static_cast<double>(baseline.filesProcessed);
    }

    if (!samples.empty())
    {
        std::sort(samples.begin(), samples.end());
        baseline.medianTimeMs = computePercentile(samples, 0.5);
        baseline.p95TimeMs = computePercentile(samples, 0.95);
    }

    if (baseline.totalTimeMs > 0.0)
    {
        double totalSeconds = baseline.totalTimeMs / 1000.0;
        baseline.throughputBytesPerSec = static_cast<size_t>(
            static_cast<double>(baseline.totalBytesProcessed) / totalSeconds);
    }

    return baseline;
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
    printf("  Median time:         %.2f ms\n", result.medianTimeMs);
    printf("  P95 time:            %.2f ms\n", result.p95TimeMs);
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
        printf("  Faster than a speeding bullet train\n");
    }
    else if (result.averageTimeMs < 50.0)
    {
        printf("  VERDICT: Good performance\n");
        printf("  Not quite lightning, but pretty fast\n");
    }
    else
    {
        printf("  VERDICT: Needs optimization\n");
        printf("  Even turtles can win races with determination\n");
    }

    printf("  ========================================\n\n");
}

void Benchmark::printCompareReport(
    const Result& fmtppResult,
    const Result& baselineResult,
    const std::string& toolName)
{
    printf("\n");
    printf("  ========================================\n");
    printf("  fmtpp Benchmark Comparison: fmtpp vs %s\n", toolName.c_str());
    printf("  ========================================\n");
    printf("  %-20s %12s %12s\n", "Metric", "fmtpp", toolName.c_str());
    printf("  %-20s %12.2f %12.2f\n", "Avg (ms)", fmtppResult.averageTimeMs, baselineResult.averageTimeMs);
    printf("  %-20s %12.2f %12.2f\n", "Min (ms)", fmtppResult.minTimeMs, baselineResult.minTimeMs);
    printf("  %-20s %12.2f %12.2f\n", "Max (ms)", fmtppResult.maxTimeMs, baselineResult.maxTimeMs);
    printf("  %-20s %12.2f %12.2f\n", "Median (ms)", fmtppResult.medianTimeMs, baselineResult.medianTimeMs);
    printf("  %-20s %12.2f %12.2f\n", "P95 (ms)", fmtppResult.p95TimeMs, baselineResult.p95TimeMs);
    printf("  %-20s %12zu %12zu\n", "Throughput (B/s)", fmtppResult.throughputBytesPerSec, baselineResult.throughputBytesPerSec);

    if (baselineResult.averageTimeMs > 0.0)
    {
        double speedup = baselineResult.averageTimeMs / fmtppResult.averageTimeMs;
        printf("\n  fmtpp is %.2fx %s than %s\n",
            speedup,
            (speedup >= 1.0) ? "faster" : "slower",
            toolName.c_str());
    }

    printf("  ========================================\n");
    printf("  May the fastest formatter win\n");
    printf("  ========================================\n\n");
}
