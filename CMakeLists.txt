cmake_minimum_required(VERSION 3.16)
project(fmtpp VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# C++11 for threading support
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++11>)

# fmtpp executable
add_executable(fmtpp
  src/main.cpp
  src/FormatterApp.cpp
  src/ASTNode.cpp
  src/ASTNodeAccept.cpp
  src/ASTNodeFactory.cpp
  src/ASTPrinter.cpp
  src/CFormatter.cpp
  src/Config.cpp
  src/ConfigBuilder.cpp
  src/CstyleFormatterBase.cpp
  src/FileHandle.cpp
  src/FormatterVisitor.cpp
  src/GoFormatter.cpp
  src/JSFormatter.cpp
  src/JavaFormatter.cpp
  src/MallocBuffer.cpp
  src/MemoryPool.cpp
  src/MmapRegion.cpp
  src/OutputBuffer.cpp
  src/Parser.cpp
  src/RustFormatter.cpp
  src/SourceBuffer.cpp
  src/TSFormatter.cpp
  src/CppFormatter.cpp
  src/CSharpFormatter.cpp
  src/StyleDetector.cpp
  src/ThreadPool.cpp
  src/TuiConfigurator.cpp
  src/Benchmark.cpp
)

target_include_directories(fmtpp PRIVATE
  ${CMAKE_SOURCE_DIR}/include
)

# Build pragma message displayed at compile time
target_compile_options(fmtpp PRIVATE
  $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-include>
  $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${CMAKE_SOURCE_DIR}/include/fmtpp/pragma.hpp>
  $<$<CXX_COMPILER_ID:MSVC>:/FI${CMAKE_SOURCE_DIR}/include/fmtpp/pragma.hpp>
)

# Static linking to avoid runtime DLL dependencies
if(WIN32)
  target_link_options(fmtpp PRIVATE -static)
  target_link_libraries(fmtpp PRIVATE ws2_32)
else()
  target_link_options(fmtpp PRIVATE -static)
endif()