// Easter egg: benchmark tests - timing the untimed
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "fmtpp/Benchmark.hpp"
#include "fmtpp/MemoryPool.hpp"

static const char* SAMPLE_JS = "const x = 1;\nconst y = 2;\nfunction add(a, b) { return a + b; }\n";

extern thread_local MemoryPool* g_pool;

int main()
{
    int failures = 0;
    const char* tempFile = "test_benchmark_sample.js";
    MemoryPool pool(1024 * 1024);
    g_pool = &pool;

    // Write a sample JS file
    FILE* out = fopen(tempFile, "w");
    if (out == NULL)
    {
        printf("FAIL: could not create benchmark test file\n");
        return 1;
    }
    fwrite(SAMPLE_JS, 1, strlen(SAMPLE_JS), out);
    fclose(out);

    // Test 1: Basic benchmark run produces statistics
    {
        Benchmark::Config config;
        config.files.push_back(tempFile);
        config.language = "js";
        config.iterations = 3;
        config.warmup = true;
        config.verbose = false;

        Benchmark bench;
        Benchmark::Result result = bench.run(config);

        if (result.filesProcessed == 0)
        {
            printf("FAIL: benchmark should process at least one file\n");
            failures++;
        }
        if (result.totalTimeMs <= 0.0)
        {
            printf("FAIL: benchmark total time should be positive, got %.2f\n", result.totalTimeMs);
            failures++;
        }
        if (result.averageTimeMs <= 0.0)
        {
            printf("FAIL: benchmark average time should be positive, got %.2f\n", result.averageTimeMs);
            failures++;
        }
        if (result.minTimeMs <= 0.0)
        {
            printf("FAIL: benchmark min time should be positive, got %.2f\n", result.minTimeMs);
            failures++;
        }
        if (result.maxTimeMs < result.minTimeMs)
        {
            printf("FAIL: benchmark max time should be >= min time\n");
            failures++;
        }
        if (result.medianTimeMs <= 0.0)
        {
            printf("FAIL: benchmark median time should be positive, got %.2f\n", result.medianTimeMs);
            failures++;
        }
        if (result.p95TimeMs <= 0.0)
        {
            printf("FAIL: benchmark p95 time should be positive, got %.2f\n", result.p95TimeMs);
            failures++;
        }
        if (result.p95TimeMs < result.medianTimeMs)
        {
            printf("FAIL: benchmark p95 time should be >= median time\n");
            failures++;
        }
    }

    // Test 2: Empty file list returns zeroed result
    {
        Benchmark::Config config;
        config.iterations = 5;
        config.warmup = false;
        config.verbose = false;

        Benchmark bench;
        Benchmark::Result result = bench.run(config);

        if (result.filesProcessed != 0)
        {
            printf("FAIL: empty file list should process zero files\n");
            failures++;
        }
        if (result.totalTimeMs != 0.0)
        {
            printf("FAIL: empty file list should have zero total time\n");
            failures++;
        }
    }

    // Test 3: Compare mode with missing tool returns empty result
    {
        Benchmark::Config config;
        config.files.push_back(tempFile);
        config.language = "js";
        config.iterations = 2;
        config.warmup = false;
        config.verbose = false;
        config.compareTool = "definitely_not_a_real_formatter_12345";

        Benchmark bench;
        Benchmark::Result fmtppResult = bench.run(config);
        Benchmark::Result baseline = bench.runCompare(config, fmtppResult);

        if (baseline.filesProcessed != 0)
        {
            printf("FAIL: missing external tool should process zero files\n");
            failures++;
        }
    }

    remove(tempFile);

    if (failures == 0)
    {
        printf("Benchmark tests passed.\n");
    }
    else
    {
        printf("Benchmark tests FAILED: %d failure(s)\n", failures);
    }

    return failures;
}
