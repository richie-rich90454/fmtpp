#pragma once
#include <vector>
#include <string>
#include <cstddef>

/**
 * Thread pool for parallel file formatting.
 * Uses C++11 std::thread with a fixed number of workers.
 * Each worker formats a file independently (no shared mutable state).
 *
 * Easter egg: the thread pool - where worker threads dream of formatted code
 */
class ThreadPool
{
public:
    /**
     * Constructs a thread pool.
     * @param numWorkers Number of worker threads (0 = hardware concurrency).
     */
    explicit ThreadPool(size_t numWorkers = 0);

    /** Destroys the thread pool, waiting for all workers. */
    ~ThreadPool();

    /**
     * Formats multiple files in parallel.
     * @param files List of file paths to format.
     * @param language Language to use for formatting ("js", "ts", "java", etc.).
     * @return Vector of formatted output strings (one per file).
     */
    std::vector<std::string> parallelFormat(
        const std::vector<std::string>& files,
        const std::string& language);

    /** Returns the number of worker threads. */
    size_t getWorkerCount() const;

private:
    ThreadPool(const ThreadPool& other);
    ThreadPool& operator=(const ThreadPool& other);

    /**
     * Formats a single file.
     * @param filepath Path to the file.
     * @param language Language identifier.
     * @return Formatted output string.
     */
    static std::string formatFile(const std::string& filepath, const std::string& language);

    size_t m_numWorkers;
};