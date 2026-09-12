#include "fmtpp/ThreadPool.hpp"
#include "fmtpp/SourceBuffer.hpp"
#include "fmtpp/Parser.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/ConfigBuilder.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/JSFormatter.hpp"
#include "fmtpp/TSFormatter.hpp"
#include "fmtpp/JavaFormatter.hpp"
#include "fmtpp/CFormatter.hpp"
#include "fmtpp/CppFormatter.hpp"
#include "fmtpp/CSharpFormatter.hpp"
#include "fmtpp/GoFormatter.hpp"
#include "fmtpp/RustFormatter.hpp"
#include <cstdio>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

// Easter egg: the thread pool awakens - parallel formatting, assemble!

// ---- Internal worker queue ----
namespace {
    struct FormatTask {
        std::string filepath;
        std::string language;
        std::string* result;
    };

    class WorkerQueue {
    public:
        WorkerQueue() : m_done(false) {}
        ~WorkerQueue() {}

        void addTask(const FormatTask& task);
        bool getTask(FormatTask& task);
        void setDone();
        bool isDone() const;

    private:
        std::queue<FormatTask> m_tasks;
        std::mutex m_mutex;
        std::condition_variable m_cond;
        bool m_done;
    };

    void WorkerQueue::addTask(const FormatTask& task) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_tasks.push(task);
        }
        m_cond.notify_one();
    }

    bool WorkerQueue::getTask(FormatTask& task) {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (!m_done && m_tasks.empty()) {
            m_cond.wait(lock);
        }
        if (m_done && m_tasks.empty()) {
            return false;
        }
        if (!m_tasks.empty()) {
            task = m_tasks.front();
            m_tasks.pop();
            return true;
        }
        return false;
    }

    void WorkerQueue::setDone() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_done = true;
        }
        m_cond.notify_all();
    }

    bool WorkerQueue::isDone() const {
        return m_done;
    }
} // anonymous namespace

// ---- ThreadPool implementation ----

ThreadPool::ThreadPool(size_t numWorkers)
    : m_numWorkers(numWorkers)
{
    if (m_numWorkers == 0)
    {
        m_numWorkers = std::thread::hardware_concurrency();
        if (m_numWorkers == 0)
        {
            m_numWorkers = 4; // Sensible default
        }
    }
}

ThreadPool::~ThreadPool()
{
    // Easter egg: thread pool shutting down - all workers clocking out
}

std::string ThreadPool::formatFile(const std::string& filepath, const std::string& language)
{
    SourceBuffer source(filepath.c_str());
    if (!source.isValid())
    {
        return std::string();
    }

    Parser parser;
    ProgramNode* ast = parser.parse(source);
    if (ast == NULL)
    {
        return std::string();
    }

    // ponytail: .fmtpp.toml next to the file wins; defaults when missing or broken
    Config config = ConfigBuilder::buildForFile(filepath);
    OutputBuffer out(static_cast<size_t>(source.getSize()) * 2);
    FormatterVisitor* formatter = NULL;

    // Select formatter based on language
    if (language == "js" || language == "javascript")
    {
        formatter = new JSFormatter(out, config);
    }
    else if (language == "ts" || language == "typescript")
    {
        formatter = new TSFormatter(out, config);
    }
    else if (language == "java")
    {
        formatter = new JavaFormatter(out, config);
    }
    else if (language == "c")
    {
        formatter = new CFormatter(out, config);
    }
    else if (language == "cpp" || language == "c++")
    {
        formatter = new CppFormatter(out, config);
    }
    else if (language == "cs" || language == "csharp" || language == "c#")
    {
        formatter = new CSharpFormatter(out, config);
    }
    else if (language == "go")
    {
        formatter = new GoFormatter(out, config);
    }
    else if (language == "rs" || language == "rust")
    {
        formatter = new RustFormatter(out, config);
    }
    else
    {
        // Default to JavaScript
        formatter = new JSFormatter(out, config);
    }

    if (formatter != NULL)
    {
        formatter->format(ast);
        std::string result = out.toString();
        delete formatter;
        return result;
    }

    return std::string();
}

std::vector<std::string> ThreadPool::parallelFormat(
    const std::vector<std::string>& files,
    const std::string& language)
{
    std::vector<std::string> results(files.size());
    std::vector<std::thread> workers;
    WorkerQueue queue;

    // Launch worker threads
    size_t i;
    for (i = 0; i < m_numWorkers; i++)
    {
        workers.push_back(std::thread([&queue]() {
            FormatTask task;
            while (queue.getTask(task))
            {
                if (task.result != NULL)
                {
                    *(task.result) = formatFile(task.filepath, task.language);
                }
            }
        }));
    }

    // Add tasks to queue
    for (i = 0; i < files.size(); i++)
    {
        FormatTask task;
        task.filepath = files[i];
        task.language = language;
        task.result = &results[i];
        queue.addTask(task);
    }

    // Signal done and wait for workers
    queue.setDone();
    for (i = 0; i < workers.size(); i++)
    {
        if (workers[i].joinable())
        {
            workers[i].join();
        }
    }

    // Easter egg: parallel formatting complete - the threads have done their work
    return results;
}

size_t ThreadPool::getWorkerCount() const
{
    return m_numWorkers;
}