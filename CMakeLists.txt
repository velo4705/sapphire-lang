cmake_minimum_required(VERSION 3.20)
project(Sapphire VERSION 1.0.0-beta.8 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_TOOLS "Build development tools" ON)
option(ENABLE_JIT "Enable JIT compilation" OFF)
option(ENABLE_PROFILING "Enable profiling support" OFF)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/runtime)

# Source files
set(COMPILER_SOURCES
    src/main.cpp
    src/lexer/lexer.cpp
    src/lexer/token.cpp
    src/parser/parser.cpp
    src/parser/ast.cpp
    src/semantic/type_checker.cpp
    src/codegen/llvm_codegen.cpp
    src/error/error.cpp
)

set(RUNTIME_SOURCES
    runtime/gc/mark_sweep.cpp
    runtime/scheduler/work_stealing.cpp
    runtime/ffi/c_abi.cpp
)

# Compiler executable
add_executable(sapphire ${COMPILER_SOURCES})
target_link_libraries(sapphire LLVM pthread)

# Runtime library
add_library(sapphire_runtime SHARED ${RUNTIME_SOURCES})

# Tools
if(BUILD_TOOLS)
    # add_subdirectory(tools/spm)
    # add_subdirectory(tools/lsp)
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    # add_subdirectory(tests)
endif()

# Install
install(TARGETS sapphire DESTINATION bin)
install(TARGETS sapphire_runtime DESTINATION lib)
install(DIRECTORY stdlib/ DESTINATION lib/sapphire/stdlib)
install(DIRECTORY examples/ DESTINATION share/sapphire/examples)
