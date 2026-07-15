// Easter egg: output buffer tests - because every byte deserves a verified home
#include <cstdio>
#include <cstring>
#include "fmtpp/OutputBuffer.hpp"

int main()
{
    int failures = 0;

    // Test 1: Default construction
    {
        OutputBuffer buf;
        if (buf.getSize() != 0)
        {
            printf("FAIL: Default size should be 0, got %zu\n", buf.getSize());
            failures++;
        }
        if (buf.getCapacity() < OutputBuffer::DEFAULT_CAPACITY)
        {
            printf("FAIL: Default capacity should be >= %zu\n", OutputBuffer::DEFAULT_CAPACITY);
            failures++;
        }
        if (buf.getData() == NULL)
        {
            printf("FAIL: getData() should not return NULL\n");
            failures++;
        }
    }

    // Test 2: append(const char*)
    {
        OutputBuffer buf;
        buf.append("Hello");
        if (buf.getSize() != 5)
        {
            printf("FAIL: append(\"Hello\") size should be 5, got %zu\n", buf.getSize());
            failures++;
        }
        if (strcmp(buf.getData(), "Hello") != 0)
        {
            printf("FAIL: append(\"Hello\") data should be \"Hello\", got \"%s\"\n", buf.getData());
            failures++;
        }
    }

    // Test 3: append(const char*, size_t)
    {
        OutputBuffer buf;
        buf.append("HelloWorld", 5); // only append "Hello"
        if (buf.getSize() != 5)
        {
            printf("FAIL: append(\"HelloWorld\", 5) size should be 5, got %zu\n", buf.getSize());
            failures++;
        }
        if (strcmp(buf.getData(), "Hello") != 0)
        {
            printf("FAIL: append(\"HelloWorld\", 5) data should be \"Hello\", got \"%s\"\n", buf.getData());
            failures++;
        }
    }

    // Test 4: appendChar
    {
        OutputBuffer buf;
        buf.appendChar('A');
        buf.appendChar('B');
        buf.appendChar('C');
        if (buf.getSize() != 3)
        {
            printf("FAIL: appendChar x3 size should be 3, got %zu\n", buf.getSize());
            failures++;
        }
        if (strcmp(buf.getData(), "ABC") != 0)
        {
            printf("FAIL: appendChar x3 data should be \"ABC\", got \"%s\"\n", buf.getData());
            failures++;
        }
    }

    // Test 5: printf
    {
        OutputBuffer buf;
        buf.printf("int=%d str=%s", 42, "world");
        if (strcmp(buf.getData(), "int=42 str=world") != 0)
        {
            printf("FAIL: printf data should be \"int=42 str=world\", got \"%s\"\n", buf.getData());
            failures++;
        }
    }

    // Test 6: toString
    {
        OutputBuffer buf;
        buf.append("ToStringTest");
        std::string s = buf.toString();
        if (s != "ToStringTest")
        {
            printf("FAIL: toString should be \"ToStringTest\", got \"%s\"\n", s.c_str());
            failures++;
        }
    }

    // Test 7: clear
    {
        OutputBuffer buf;
        buf.append("something");
        buf.clear();
        if (buf.getSize() != 0)
        {
            printf("FAIL: clear() size should be 0, got %zu\n", buf.getSize());
            failures++;
        }
        if (buf.getData()[0] != '\0')
        {
            printf("FAIL: clear() first byte should be '\\0', got '%c'\n", buf.getData()[0]);
            failures++;
        }
    }

    // Test 8: Multiple appends
    {
        OutputBuffer buf;
        buf.append("Hello");
        buf.appendChar(' ');
        buf.append("World");
        buf.appendChar('!');
        if (strcmp(buf.getData(), "Hello World!") != 0)
        {
            printf("FAIL: multiple appends should be \"Hello World!\", got \"%s\"\n", buf.getData());
            failures++;
        }
        if (buf.getSize() != 12)
        {
            printf("FAIL: multiple appends size should be 12, got %zu\n", buf.getSize());
            failures++;
        }
    }

    // Test 9: Dynamic growth (append beyond initial capacity)
    {
        OutputBuffer buf(16); // small initial capacity
        const char* longStr = "This is a long string that should exceed the initial capacity of 16 bytes";
        buf.append(longStr);
        if (strcmp(buf.getData(), longStr) != 0)
        {
            printf("FAIL: dynamic growth data mismatch\n");
            failures++;
        }
        if (buf.getSize() != strlen(longStr))
        {
            printf("FAIL: dynamic growth size should be %zu, got %zu\n", strlen(longStr), buf.getSize());
            failures++;
        }
    }

    // Test 10: printf with dynamic growth
    {
        OutputBuffer buf(16);
        buf.printf("This is a formatted string with %d and %s that should grow", 12345, "value");
        const char* expected = "This is a formatted string with 12345 and value that should grow";
        if (strcmp(buf.getData(), expected) != 0)
        {
            printf("FAIL: printf dynamic growth data mismatch\n");
            printf("  Expected: \"%s\"\n", expected);
            printf("  Got:      \"%s\"\n", buf.getData());
            failures++;
        }
    }

    // Test 11: flushTo
    {
        OutputBuffer buf;
        buf.append("FlushTest");
        FILE* tmp = fopen("test_flush_output.txt", "w");
        if (tmp != NULL)
        {
            buf.flushTo(tmp);
            fclose(tmp);
            // Read back and verify
            FILE* in = fopen("test_flush_output.txt", "r");
            if (in != NULL)
            {
                char readback[64] = {0};
                fread(readback, 1, sizeof(readback) - 1, in);
                fclose(in);
                if (strcmp(readback, "FlushTest") != 0)
                {
                    printf("FAIL: flushTo readback should be \"FlushTest\", got \"%s\"\n", readback);
                    failures++;
                }
            }
            remove("test_flush_output.txt");
        }
    }

    if (failures == 0)
    {
        printf("OutputBuffer tests passed.\n");
    }
    else
    {
        printf("OutputBuffer tests FAILED: %d failure(s)\n", failures);
    }

    return failures;
}