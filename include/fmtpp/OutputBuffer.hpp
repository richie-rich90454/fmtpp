#pragma once
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>
#include <cstdarg>

/**
 * Direct output buffer for formatted code.
 * Wraps a char* buffer that grows dynamically using realloc.
 * Provides printf-like formatting and direct append operations.
 * No std::string concatenation - writes directly into the buffer.
 *
 * Easter egg: the output buffer - where formatted code comes to life, one byte at a time
 */
class OutputBuffer
{
public:
    static const size_t DEFAULT_CAPACITY = 4096;

    /**
     * Constructs an output buffer with the given initial capacity.
     * @param capacity Initial buffer size in bytes.
     */
    explicit OutputBuffer(size_t capacity = DEFAULT_CAPACITY);

    /** Frees the internal buffer. */
    ~OutputBuffer();

    // Non-copyable
    OutputBuffer(const OutputBuffer& other);
    OutputBuffer& operator=(const OutputBuffer& other);

    /** Append a string of known length. */
    void append(const char* s, size_t len);

    /** Append a null-terminated string. */
    void append(const char* s);

    /** Append a single character. */
    void appendChar(char c);

    /** printf-style formatted append using vsnprintf. */
    void printf(const char* format, ...);

    /** Returns a pointer to the internal buffer data. */
    const char* getData() const;

    /** Returns the current size of the buffer. */
    size_t getSize() const;

    /** Returns the current capacity of the buffer. */
    size_t getCapacity() const;

    /** Converts the buffer content to a std::string. */
    std::string toString() const;

    /** Clears the buffer content (keeps allocated memory). */
    void clear();

    /** Writes the buffer content to a FILE stream. */
    void flushTo(FILE* file);

private:
    /** Ensures the buffer has enough capacity for 'additional' bytes. */
    void ensureCapacity(size_t additional);

    char* m_buffer;
    size_t m_size;
    size_t m_capacity;
};