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
        size_t filesProcessed;
        size_t totalBytesProcessed;
        size_t throughputBytesPerSec;
        size_t memoryPoolUsed;
        size_t memoryPoolCapacity;
    };

    /** Configuration for benchmark runs. */
    struct Config
    {
        int iterations;
        bool warmup;
        bool verbose;
        std::string language;
        std::vector<std::string> files;
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

private:
    Benchmark(const Benchmark& other);
    Benchmark& operator=(const Benchmark& other);

    /** Formats a single file and returns the time taken. */
    double formatFile(const std::string& filepath, const std::string& language);

    /** Gets the current time in milliseconds. */
    double getTimeMs();

    // Easter egg: the benchmarker - measuring the unmeasurable
};