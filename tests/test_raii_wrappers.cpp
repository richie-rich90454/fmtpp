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

    // Test 4: FileHandle move semantics
    {
        const char* testPath = "test_raii_move.txt";
        FILE* f = fopen(testPath, "w");
        if (f != NULL)
        {
            fprintf(f, "move");
            fclose(f);
        }

        FileHandle original(testPath, "r");
        FileHandle moved(static_cast<FileHandle&&>(original));

        if (original.isValid())
        {
            printf("FAIL: moved-from FileHandle should be invalid\n");
            failures++;
        }
        else if (!moved.isValid())
        {
            printf("FAIL: moved-to FileHandle should be valid\n");
            failures++;
        }
        else
        {
            char buffer[16] = {0};
            fread(buffer, 1, sizeof(buffer) - 1, moved.get());
            if (strcmp(buffer, "move") != 0)
            {
                printf("FAIL: FileHandle move lost data\n");
                failures++;
            }
            else
            {
                printf("FileHandle move semantics: PASSED\n");
            }
        }

        remove(testPath);
    }

    // Test 5: MallocBuffer move semantics
    {
        MallocBuffer original(64);
        char* data = static_cast<char*>(original.get());
        strcpy(data, "move");
        MallocBuffer moved(static_cast<MallocBuffer&&>(original));

        if (original.isValid())
        {
            printf("FAIL: moved-from MallocBuffer should be invalid\n");
            failures++;
        }
        else if (!moved.isValid())
        {
            printf("FAIL: moved-to MallocBuffer should be valid\n");
            failures++;
        }
        else if (strcmp(static_cast<char*>(moved.get()), "move") != 0)
        {
            printf("FAIL: MallocBuffer move lost data\n");
            failures++;
        }
        else
        {
            printf("MallocBuffer move semantics: PASSED\n");
        }
    }

    // Test 6: MmapRegion move semantics
    {
        const char* testPath = "test_raii_mmap_move.txt";
        FILE* f = fopen(testPath, "w");
        if (f != NULL)
        {
            fprintf(f, "mmapmove");
            fclose(f);
        }

        MmapRegion original(testPath);
        MmapRegion moved(static_cast<MmapRegion&&>(original));

        if (original.isValid())
        {
            printf("FAIL: moved-from MmapRegion should be invalid\n");
            failures++;
        }
        else if (!moved.isValid())
        {
            printf("FAIL: moved-to MmapRegion should be valid\n");
            failures++;
        }
        else
        {
            const char* data = static_cast<const char*>(moved.getData());
            if (data == NULL || strncmp(data, "mmapmove", 8) != 0)
            {
                printf("FAIL: MmapRegion move lost data\n");
                failures++;
            }
            else
            {
                printf("MmapRegion move semantics: PASSED\n");
            }
        }

        remove(testPath);
    }

    // Test 7: Default/null FileHandle destructor does not crash
    {
        FileHandle nullHandle(NULL, NULL);
        if (!nullHandle.isValid())
        {
            printf("Null FileHandle destructor: PASSED\n");
        }
        else
        {
            printf("FAIL: null FileHandle should be invalid\n");
            failures++;
        }
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
