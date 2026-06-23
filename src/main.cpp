#include "fmtpp/FormatterApp.hpp"
#include "fmtpp/MemoryPool.hpp"

// Global memory pool for AST nodes - ensures zero leaks via RAII
// Easter egg: the pool that never forgets to clean up
static MemoryPool g_memoryPool(1024 * 1024); // 1MB pool

// Easter egg: simple entry point, just enough to get the party started
int main(int argc, const char* argv[])
{
    FormatterApp app;
    return app.run(argc, argv);
}