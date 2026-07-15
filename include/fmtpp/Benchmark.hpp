#pragma once
#include <string>
#include <vector>
#include <cstddef>

/**
 * Benchmark utility for measuring fmtpp formatting performance.
 * Runs multiple iterations and reports average time, memory usage,
 * and throughput statistics.
 *
 * Easter egg: the benchmarker - counting nanoseconds so you don't have to
 */
class Benchmark
{
public:
    /** Result of a single benchmark run. */
    struct Result
    {
        double totalTimeMs;
        double averageTimeMs;
        double minTimeMs;
        double maxTimeMs;
        double medianTimeMs;
        double p95TimeMs;
        size_t filesProcessed;
        size_t totalBytesProcessed;
        size_t throughputBytesPerSec;
        size_t memoryPoolUsed;
        size_t memoryPoolCapacity;

        Result()
            : totalTimeMs(0.0)
            , averageTimeMs(0.0)
            , minTimeMs(0.0)
            , maxTimeMs(0.0)
            , medianTimeMs(0.0)
            , p95TimeMs(0.0)
            , filesProcessed(0)
            , totalBytesProcessed(0)
            , throughputBytesPerSec(0)
            , memoryPoolUsed(0)
            , memoryPoolCapacity(0)
        {
        }
    };

    /** Configuration for benchmark runs. */
    struct Config
    {
        int iterations;
        bool warmup;
        bool verbose;
        std::string language;
        std::vector<std::string> files;
        std::string compareTool;

        Config()
            : iterations(10)
            , warmup(true)
            , verbose(false)
            , language("js")
            , compareTool()
        {
        }
    };

    Benchmark();
    ~Benchmark();

    /**
     * Runs a benchmark with the given configuration.
     * @param config Benchmark configuration.
     * @return Benchmark result.
     */
    Result run(const Config& config);

    /**
     * Prints a formatted benchmark report.
     * @param result The benchmark result to print.
     */
    void printReport(const Result& result);

    /**
     * Runs a comparison benchmark between fmtpp and an external formatter.
     * @param config Benchmark configuration.
     * @param fmtppResult Result of the fmtpp benchmark.
     * @return Result of the external tool benchmark.
     */
    Result runCompare(const Config& config, const Result& fmtppResult);

    /**
     * Prints a comparison report between fmtpp and an external tool.
     * @param fmtppResult Result of the fmtpp benchmark.
     * @param baselineResult Result of the external tool benchmark.
     * @param toolName Name of the external tool.
     */
    void printCompareReport(
        const Result& fmtppResult,
        const Result& baselineResult,
        const std::string& toolName);

private:
    Benchmark(const Benchmark& other);
    Benchmark& operator=(const Benchmark& other);

    /** Formats a single file and returns the time taken. */
    double formatFile(const std::string& filepath, const std::string& language);

    /** Formats a single file with an external tool and returns the time taken. */
    double formatFileExternal(const std::string& filepath, const std::string& tool);

    /** Gets the current time in milliseconds. */
    double getTimeMs();

    /**
     * Computes the kth percentile from a sorted list of samples.
     * @param samples Sorted vector of timing samples.
     * @param percentile Percentile to compute (0.0 - 1.0).
     * @return The percentile value.
     */
    double computePercentile(const std::vector<double>& samples, double percentile) const;

    // Easter egg: the benchmarker - measuring the unmeasurable
};