#pragma once
#include <cstdio>

/**
 * RAII wrapper for FILE* handles.
 * Opens a file with fopen in the constructor and closes in the destructor.
 * Non-copyable.
 *
 * Easter egg: the file handle - opening and closing files since 1972
 */
class FileHandle
{
public:
    /**
     * Opens a file.
     * @param filepath Path to the file.
     * @param mode Open mode (e.g., "r", "w", "rb").
     */
    FileHandle(const char* filepath, const char* mode);

    /** Closes the file handle. */
    ~FileHandle();

    // Non-copyable
    FileHandle(const FileHandle& other);
    FileHandle& operator=(const FileHandle& other);

    /** Returns the underlying FILE pointer. */
    FILE* get() const;

    /** Returns true if the file was opened successfully. */
    bool isValid() const;

private:
    FILE* m_file;
};