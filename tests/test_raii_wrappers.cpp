#include <cstdio>
#include <cstring>
#include "fmtpp/FileHandle.hpp"
#include "fmtpp/MallocBuffer.hpp"
#include "fmtpp/MmapRegion.hpp"

// Easter egg: RAII wrappers - because manual cleanup is for mortals

int main()
{
    int failures = 0;

    // Test 1: FileHandle opens and closes a file
    {
        const char* testPath = "test_raii_file.txt";
        FILE* f = fopen(testPath, "w");
        if (f != NULL)
        {
            fprintf(f, "hello");
            fclose(f);
        }

        FileHandle handle(testPath, "r");
        if (!handle.isValid())
        {
            printf("FAIL: FileHandle should be valid for existing file\n");
            failures++;
        }
        else
        {
            char buffer[16] = {0};
            FILE* file = handle.get();
            size_t n = fread(buffer, 1, sizeof(buffer) - 1, file);
            if (strcmp(buffer, "hello") != 0)
            {
                printf("FAIL: FileHandle read wrong content: '%s' (read %zu bytes)\n", buffer, n);
                failures++;
            }
            else
            {
                printf("FileHandle open/read/close: PASSED\n");
            }
        }

        remove(testPath);
    }

    // Test 2: MallocBuffer allocates and frees memory
    {
        MallocBuffer buf(64);
        if (!buf.isValid())
        {
            printf("FAIL: MallocBuffer should be valid\n");
            failures++;
        }
        else
        {
            char* data = static_cast<char*>(buf.get());
            strcpy(data, "RAII");
            if (strcmp(data, "RAII") != 0)
            {
                printf("FAIL: MallocBuffer data mismatch\n");
                failures++;
            }
            else
            {
                printf("MallocBuffer allocate/free: PASSED\n");
            }
        }
    }

    // Test 3: MmapRegion maps and unmaps a file
    {
        const char* testPath = "test_raii_mmap.txt";
        FILE* f = fopen(testPath, "w");
        if (f != NULL)
        {
            fprintf(f, "mapped");
            fclose(f);
        }

        MmapRegion region(testPath);
        if (!region.isValid())
        {
            printf("FAIL: MmapRegion should be valid\n");
            failures++;
        }
        else
        {
            const char* data = static_cast<const char*>(region.getData());
            if (data == NULL || strncmp(data, "mapped", 6) != 0)
            {
                printf("FAIL: MmapRegion data mismatch\n");
                failures++;
            }
            else
            {
                printf("MmapRegion map/unmap: PASSED\n");
            }
        }

        remove(testPath);
    }

    if (failures == 0)
    {
        printf("All RAII wrapper tests passed.\n");
    }
    else
    {
        printf("RAII wrapper tests FAILED: %d failure(s)\n", failures);
    }

    return failures;
}
