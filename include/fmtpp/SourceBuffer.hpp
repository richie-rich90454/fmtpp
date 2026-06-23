#pragma once
#include <cstddef>

/**
 * RAII wrapper for source file content.
 * Loads a file into memory using fread (all platforms).
 * The source buffer must outlive all ASTNode objects created from it.
 * Non-copyable.
 */
class SourceBuffer
{
public:
    /**
     * Loads a file from the given path.
     * Uses fread to read the file content into a malloc'd buffer.
     * @param filepath Path to the source file.
     */
    explicit SourceBuffer(const char* filepath);

    /** Unmaps or frees the source buffer. */
    ~SourceBuffer();

    // Non-copyable
    SourceBuffer(const SourceBuffer& other);
    SourceBuffer& operator=(const SourceBuffer& other);

    /** Returns a pointer to the source text. */
    const char* getData() const;

    /** Returns the size of the source text in bytes. */
    size_t getSize() const;

    /** Returns true if the file was loaded successfully. */
    bool isValid() const;

private:
    char* m_data;
    size_t m_size;
    bool m_isValid;
    bool m_isMapped;
    // Easter egg: the source code, fresh from disk, ready for transformation
};