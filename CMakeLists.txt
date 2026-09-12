cmake_minimum_required(VERSION 3.28)
project(fmtpp VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Optional undefined-behavior sanitizer for QA (phase 23)
option(FMTPP_SANITIZE_UB "Build with undefined behavior sanitizer" OFF)
if(FMTPP_SANITIZE_UB)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()

# Third-party dependencies.
# CLI11 is header-only: vendored as bare headers under lib/cli11 (v2.7.2,
# BSD-3-Clause, see lib/cli11/LICENSE). No FetchContent needed for it.
add_library(cli11_headers INTERFACE)
target_include_directories(cli11_headers INTERFACE ${CMAKE_SOURCE_DIR}/lib/cli11/include)
add_library(CLI11::CLI11 ALIAS cli11_headers)

# toml++ via FetchContent; the local mirror is used when network
# FetchContent is unavailable (override with FETCHCONTENT_SOURCE_DIR_TOMLPLUSPLUS).
include(FetchContent)
set(FETCHCONTENT_SOURCE_DIR_TOMLPLUSPLUS ${CMAKE_SOURCE_DIR}/lib/tomlplusplus CACHE PATH "toml++ source directory")

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

# Toolchain feature test: toml++ uses SSE2 intrinsics for UTF-8 validation on
# x86-64. Some Clang/MSVC combinations emit real calls to those intrinsics
# instead of inlining them, which then fail to link (the symbols live in
# compiler headers, not in any linkable library). Detect a working
# compile+link once; only toolchains that fail fall back to toml++'s scalar
# path. Working toolchains (all common OSes/arches) keep SIMD and are
# unaffected. Both macros are #ifndef-guarded in toml++, so -D overrides them.
include(CheckCXXSourceCompiles)
check_cxx_source_compiles(
  "#include <emmintrin.h>
   int main() { __m128i v = _mm_setzero_si128(); (void)v; return 0; }"
  FMTPP_HAVE_WORKING_SSE2)
if(NOT FMTPP_HAVE_WORKING_SSE2)
  message(STATUS "fmtpp: toolchain cannot link SSE2 intrinsics; using scalar toml++ path")
  add_compile_definitions(TOML_ENABLE_SIMD=0)
endif()

set(FMTPP_SOURCES
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
  src/KotlinFormatter.cpp
  src/SwiftFormatter.cpp
  src/DartFormatter.cpp
  src/PhpFormatter.cpp
  src/ScalaFormatter.cpp
  src/ObjectiveCFormatter.cpp
  src/ZigFormatter.cpp
  src/DFormatter.cpp
  src/GroovyFormatter.cpp
  src/SolidityFormatter.cpp
  src/VFormatter.cpp
  src/GlslFormatter.cpp
  src/HtmlFormatter.cpp
  src/CssFormatter.cpp
  src/VueFormatter.cpp
  src/FormatterFactory.cpp
  src/StyleDetector.cpp
  src/ThreadPool.cpp
  src/TuiConfigurator.cpp
  src/Benchmark.cpp
)

add_library(fmtpp_static STATIC ${FMTPP_SOURCES})

# toml++ v3 requires C++17; override the project default for these files only.
# Generator expressions keep this portable: -std= for GCC/Clang, /std: for MSVC.
set_source_files_properties(
  src/ConfigBuilder.cpp
  src/StyleDetector.cpp
  src/TuiConfigurator.cpp
  PROPERTIES COMPILE_OPTIONS "$<$<CXX_COMPILER_ID:MSVC>:/std:c++17>$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-std=c++17>"
)

target_include_directories(fmtpp_static PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(fmtpp_static PUBLIC CLI11::CLI11 tomlplusplus::tomlplusplus)

# Build pragma message displayed at compile time
target_compile_options(fmtpp_static PUBLIC
  $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-include>
  $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${CMAKE_SOURCE_DIR}/include/fmtpp/pragma.hpp>
  $<$<CXX_COMPILER_ID:MSVC>:/FI${CMAKE_SOURCE_DIR}/include/fmtpp/pragma.hpp>
)

# fmtpp executable
add_executable(fmtpp src/main.cpp)
target_link_libraries(fmtpp PRIVATE fmtpp_static)

# Static linking to avoid runtime DLL dependencies.
# Portable across all target OSes:
# - Linux: fully-static link is supported -> use -static.
# - Windows/MinGW: static libstdc++/libgcc only (fully-static PE is unsupported).
# - macOS and Windows/MSVC: no extra flags (fully-static binaries unsupported).
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(fmtpp PRIVATE -static)
elseif(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_options(fmtpp PRIVATE -static-libstdc++ -static-libgcc)
endif()
if(WIN32)
  target_link_libraries(fmtpp PRIVATE ws2_32)
endif()

# Tests
enable_testing()

set(FMTPP_TESTS
  test_ast
  test_benchmark
  test_cformatter
  test_configbuilder
  test_cppformatter
  test_csharpformatter
  test_cstyle_formatter_base_core
  test_cssformatter
  test_dartformatter
  test_dformatter
  test_formatter_visitor
  test_goformatter
  test_groovyformatter
  test_glslformatter
  test_htmlformatter
  test_javaformatter
  test_jsformatter
  test_kotlinformatter
  test_memorypool_astnode
  test_objectivecformatter
  test_outputbuffer
  test_parser
  test_phpformatter
  test_raii_wrappers
  test_rustformatter
  test_scalaformatter
  test_solidityformatter
  test_swiftformatter
  test_threadpool
  test_tsformatter
  test_vformatter
  test_vueformatter
  test_zigformatter
)

foreach(test_name IN LISTS FMTPP_TESTS)
  add_executable(${test_name} tests/${test_name}.cpp)
  target_link_libraries(${test_name} PRIVATE fmtpp_static)
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(${test_name} PRIVATE -static)
  elseif(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(${test_name} PRIVATE -static-libstdc++ -static-libgcc)
  endif()
  if(WIN32)
    target_link_libraries(${test_name} PRIVATE ws2_32)
  endif()
  add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
