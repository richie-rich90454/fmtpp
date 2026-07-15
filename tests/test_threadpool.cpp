// Easter egg: thread pool tests - many threads, one dream
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include "fmtpp/ThreadPool.hpp"
#include "fmtpp/MemoryPool.hpp"

extern MemoryPool* g_pool;

static void writeFile(const char* path, const char* content)
{
    FILE* f = fopen(path, "w");
    if (f != NULL)
    {
        fwrite(content, 1, strlen(content), f);
        fclose(f);
    }
}

int main()
{
    int failures = 0;
    MemoryPool pool(2 * 1024 * 1024);
    g_pool = &pool;

    const char* fileA = "test_threadpool_a.js";
    const char* fileB = "test_threadpool_b.js";
    const char* fileC = "test_threadpool_c.js";

    writeFile(fileA, "const a = 1;");
    writeFile(fileB, "const b = 2;");
    writeFile(fileC, "const c = 3;");

    // Test 1: Worker count is positive
    {
        ThreadPool pool(0);
        if (pool.getWorkerCount() == 0)
        {
            printf("FAIL: thread pool should have at least one worker\n");
            failures++;
        }
    }

    // Test 2: parallelFormat returns results in submission order
    {
        ThreadPool pool(2);
        std::vector<std::string> files;
        files.push_back(fileA);
        files.push_back(fileB);
        files.push_back(fileC);

        std::vector<std::string> results = pool.parallelFormat(files, "js");

        if (results.size() != files.size())
        {
            printf("FAIL: parallelFormat result count should be %zu, got %zu\n",
                files.size(), results.size());
            failures++;
        }

        for (size_t i = 0; i < results.size(); i++)
        {
            if (results[i].empty())
            {
                printf("FAIL: parallelFormat result[%zu] should not be empty\n", i);
                failures++;
            }
        }
    }

    // Test 3: Single file parallel format works
    {
        ThreadPool pool(2);
        std::vector<std::string> files;
        files.push_back(fileA);

        std::vector<std::string> results = pool.parallelFormat(files, "js");

        if (results.size() != 1)
        {
            printf("FAIL: single file parallelFormat should return 1 result\n");
            failures++;
        }
        else if (results[0].empty())
        {
            printf("FAIL: single file parallelFormat result should not be empty\n");
            failures++;
        }
    }

    // Test 4: Empty file list returns empty results
    {
        ThreadPool pool(2);
        std::vector<std::string> files;
        std::vector<std::string> results = pool.parallelFormat(files, "js");

        if (!results.empty())
        {
            printf("FAIL: empty file list should return empty results\n");
            failures++;
        }
    }

    remove(fileA);
    remove(fileB);
    remove(fileC);

    if (failures == 0)
    {
        printf("ThreadPool tests passed.\n");
    }
    else
    {
        printf("ThreadPool tests FAILED: %d failure(s)\n", failures);
    }

    return failures;
}
