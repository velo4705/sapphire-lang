# Sapphire Makefile
# Quick build system (use CMake for full build)

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -I./src
LDFLAGS = -lpthread

# Check for LLVM (optional for now)
LLVM_CONFIG := $(shell command -v llvm-config 2> /dev/null)
ifdef LLVM_CONFIG
    LLVM_CXXFLAGS := $(shell llvm-config --cxxflags)
    LLVM_LDFLAGS := $(shell llvm-config --ldflags --libs core support irreader passes target x86)
    CXXFLAGS += $(LLVM_CXXFLAGS) -DHAVE_LLVM
    LDFLAGS += $(LLVM_LDFLAGS)
    LLVM_STATUS = "✓ LLVM support enabled"
else
    LLVM_STATUS = "⚠ LLVM not found - code generation disabled"
endif

TARGET = sapp
ALIAS = sapphire
SRCDIR = src
OBJDIR = build/obj

# Source files
SOURCES = $(SRCDIR)/main.cpp \
          $(SRCDIR)/lexer/lexer.cpp \
          $(SRCDIR)/lexer/token.cpp \
          $(SRCDIR)/parser/parser.cpp \
          $(SRCDIR)/parser/expr.cpp \
          $(SRCDIR)/parser/stmt.cpp \
          $(SRCDIR)/interpreter/interpreter.cpp \
          $(SRCDIR)/repl/repl.cpp \
          $(SRCDIR)/types/type.cpp \
          $(SRCDIR)/semantic/type_checker.cpp \
          $(SRCDIR)/semantic/type_inference.cpp

# Add codegen if LLVM is available
ifdef LLVM_CONFIG
    SOURCES += $(SRCDIR)/codegen/llvm_codegen.cpp \
               $(SRCDIR)/codegen/codegen_impl.cpp
endif

OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

.PHONY: all build test install clean help run test-allocator test-gc test-safety test-refcount test-ownership test-profiler test-stdlib test-runtime

all: help

test-allocator: runtime/allocator.cpp runtime/allocator.h runtime/safety.cpp runtime/safety.h tests/runtime/test_allocator.cpp
	@echo "Building allocator test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. runtime/allocator.cpp runtime/safety.cpp tests/runtime/test_allocator.cpp -o build/tests/test_allocator -lpthread
	@echo "✓ Test built: build/tests/test_allocator"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_allocator

test-gc: runtime/allocator.cpp runtime/allocator.h runtime/safety.cpp runtime/safety.h runtime/gc.cpp runtime/gc.h tests/runtime/test_gc.cpp
	@echo "Building GC test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. runtime/allocator.cpp runtime/safety.cpp runtime/gc.cpp tests/runtime/test_gc.cpp -o build/tests/test_gc -lpthread
	@echo "✓ Test built: build/tests/test_gc"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_gc

test-safety: runtime/allocator.cpp runtime/allocator.h runtime/safety.cpp runtime/safety.h tests/runtime/test_safety.cpp
	@echo "Building safety test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. runtime/allocator.cpp runtime/safety.cpp tests/runtime/test_safety.cpp -o build/tests/test_safety -lpthread
	@echo "✓ Test built: build/tests/test_safety"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_safety

test-refcount: runtime/refcount.cpp runtime/refcount.h tests/runtime/test_refcount.cpp
	@echo "Building refcount test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. runtime/refcount.cpp tests/runtime/test_refcount.cpp -o build/tests/test_refcount -lpthread
	@echo "✓ Test built: build/tests/test_refcount"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_refcount

test-ownership: runtime/ownership.cpp runtime/ownership.h tests/runtime/test_ownership.cpp
	@echo "Building ownership test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. runtime/ownership.cpp tests/runtime/test_ownership.cpp -o build/tests/test_ownership -lpthread
	@echo "✓ Test built: build/tests/test_ownership"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_ownership

test-profiler: runtime/profiler.cpp runtime/profiler.h tests/runtime/test_profiler.cpp
	@echo "Building profiler test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. runtime/profiler.cpp tests/runtime/test_profiler.cpp -o build/tests/test_profiler -lpthread
	@echo "✓ Test built: build/tests/test_profiler"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_profiler

test-stdlib: stdlib/core/string.cpp stdlib/io/file.cpp stdlib/math/math.cpp stdlib/tests/test_stdlib.cpp
	@echo "Building stdlib test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. stdlib/core/string.cpp stdlib/io/file.cpp stdlib/math/math.cpp stdlib/tests/test_stdlib.cpp -o build/tests/test_stdlib -lpthread
	@echo "✓ Test built: build/tests/test_stdlib"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_stdlib

test-json: stdlib/json/json.cpp stdlib/tests/test_json.cpp
	@echo "Building JSON test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. stdlib/json/json.cpp stdlib/tests/test_json.cpp -o build/tests/test_json
	@echo "✓ Test built: build/tests/test_json"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_json

test-base64: stdlib/encoding/base64.cpp stdlib/tests/test_base64.cpp
	@echo "Building Base64 test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. stdlib/encoding/base64.cpp stdlib/tests/test_base64.cpp -o build/tests/test_base64
	@echo "✓ Test built: build/tests/test_base64"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_base64

test-cli: stdlib/cli/argparser.cpp stdlib/tests/test_cli.cpp
	@echo "Building CLI test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. stdlib/cli/argparser.cpp stdlib/tests/test_cli.cpp -o build/tests/test_cli
	@echo "✓ Test built: build/tests/test_cli"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_cli

test-domain: test-json test-base64 test-cli
	@echo ""
	@echo "================================================================================";
	@echo "All domain library tests passed! 📚✅"
	@echo "================================================================================";

test-thread: runtime/concurrency/thread.cpp tests/runtime/test_thread.cpp
	@echo "Building thread test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. runtime/concurrency/thread.cpp tests/runtime/test_thread.cpp -o build/tests/test_thread -lpthread
	@echo "✓ Test built: build/tests/test_thread"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_thread

test-mutex: runtime/concurrency/mutex.cpp tests/runtime/test_mutex.cpp
	@echo "Building mutex test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. runtime/concurrency/mutex.cpp runtime/concurrency/thread.cpp tests/runtime/test_mutex.cpp -o build/tests/test_mutex -lpthread
	@echo "✓ Test built: build/tests/test_mutex"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_mutex

test-rwlock: runtime/concurrency/rwlock.cpp tests/runtime/test_rwlock.cpp
	@echo "Building rwlock test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. runtime/concurrency/rwlock.cpp runtime/concurrency/thread.cpp tests/runtime/test_rwlock.cpp -o build/tests/test_rwlock -lpthread
	@echo "✓ Test built: build/tests/test_rwlock"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_rwlock

test-channel: tests/runtime/test_channel.cpp
	@echo "Building channel test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. runtime/concurrency/thread.cpp tests/runtime/test_channel.cpp -o build/tests/test_channel -lpthread
	@echo "✓ Test built: build/tests/test_channel"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_channel

test-threadpool: runtime/concurrency/threadpool.cpp tests/runtime/test_threadpool.cpp
	@echo "Building threadpool test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. runtime/concurrency/threadpool.cpp tests/runtime/test_threadpool.cpp -o build/tests/test_threadpool -lpthread
	@echo "✓ Test built: build/tests/test_threadpool"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_threadpool

test-concurrency: test-thread test-mutex test-rwlock test-channel test-threadpool
	@echo ""
	@echo "================================================================================";
	@echo "All concurrency tests passed! ⚡✅"
	@echo "================================================================================";

test-runtime: test-allocator test-gc test-safety test-refcount test-ownership test-profiler
	@echo ""
	@echo "================================================================================";
	@echo "All runtime tests passed! ✅"
	@echo "================================================================================";

help:
	@echo "Sapphire Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make quick      - Quick build (no CMake)"
	@echo "  make spm        - Build package manager"
	@echo "  make run        - Build and run hello.spp"
	@echo "  make clean      - Clean build artifacts"
	@echo ""
	@echo "Status:"
	@echo "  $(LLVM_STATUS)"
	@echo ""
	@echo "For full build, use: ./scripts/build.sh"

spm:
	@echo "Building spm (Sapphire Package Manager)..."
	@mkdir -p build/spm
	$(CXX) -std=c++20 -Wall -Wextra -O2 \
		tools/spm/main.cpp \
		-o spm
	@echo "✓ Build complete: ./spm"

sapphire-fmt:
	@echo "Building sapphire-fmt (Code Formatter)..."
	$(CXX) -std=c++20 -Wall -Wextra -O2 \
		tools/fmt/main.cpp \
		src/formatter/formatter.cpp \
		-o sapphire-fmt
	@echo "✓ Build complete: ./sapphire-fmt"

sapphire-lsp:
	@echo "Building sapphire-lsp (Language Server)..."
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I./src \
		tools/lsp/main.cpp \
		src/lexer/lexer.cpp \
		src/lexer/token.cpp \
		src/parser/parser.cpp \
		src/parser/expr.cpp \
		src/parser/stmt.cpp \
		src/types/type.cpp \
		-o sapphire-lsp
	@echo "✓ Build complete: ./sapphire-lsp"

sapphire-debug:
	@echo "Building sapphire-debug (Debug Adapter)..."
	$(CXX) -std=c++20 -Wall -Wextra -O2 \
		tools/debug/main.cpp \
		-o sapphire-debug
	@echo "✓ Build complete: ./sapphire-debug"

tools: spm sapphire-fmt sapphire-lsp sapphire-debug
	@echo "✓ All tools built successfully!"

quick: $(TARGET)
	@ln -sf $(TARGET) $(ALIAS)
	@echo "✓ Build complete: ./$(TARGET)"
	@echo "  Alias created: ./$(ALIAS) -> ./$(TARGET)"
	@echo ""
	@echo "$(LLVM_STATUS)"

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/lexer
	mkdir -p $(OBJDIR)/parser
	mkdir -p $(OBJDIR)/interpreter
	mkdir -p $(OBJDIR)/types
	mkdir -p $(OBJDIR)/semantic
	mkdir -p $(OBJDIR)/error
	mkdir -p $(OBJDIR)/codegen

run: quick
	@echo "Running examples/hello.spp..."
	@./sapp examples/hello.spp

clean:
	rm -rf $(OBJDIR) $(TARGET) $(ALIAS)
	rm -rf build/



test-examples:
	@bash test_examples.sh


test-datetime: stdlib/datetime/datetime.cpp stdlib/tests/test_datetime.cpp
	@echo "Building datetime test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. stdlib/datetime/datetime.cpp stdlib/tests/test_datetime.cpp -o build/tests/test_datetime
	@echo "✓ Test built: build/tests/test_datetime"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_datetime

test-system: stdlib/system/system.cpp stdlib/tests/test_system.cpp
	@echo "Building system test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. stdlib/system/system.cpp stdlib/tests/test_system.cpp -o build/tests/test_system -lpthread
	@echo "✓ Test built: build/tests/test_system"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_system

test-graphics: stdlib/graphics/graphics.cpp stdlib/tests/test_graphics.cpp
	@echo "Building graphics test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. stdlib/graphics/graphics.cpp stdlib/tests/test_graphics.cpp -o build/tests/test_graphics
	@echo "✓ Test built: build/tests/test_graphics"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_graphics

test-utils: stdlib/utils/utils.cpp stdlib/tests/test_utils.cpp
	@echo "Building utils test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. stdlib/utils/utils.cpp stdlib/tests/test_utils.cpp -o build/tests/test_utils
	@echo "✓ Test built: build/tests/test_utils"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_utils

test-algorithms: stdlib/algorithms/algorithms.cpp stdlib/tests/test_algorithms.cpp
	@echo "Building algorithms test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. stdlib/algorithms/algorithms.cpp stdlib/tests/test_algorithms.cpp -o build/tests/test_algorithms
	@echo "✓ Test built: build/tests/test_algorithms"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_algorithms

test-window: stdlib/gui/window.cpp stdlib/tests/test_window.cpp
	@echo "Building window test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. stdlib/gui/window.cpp stdlib/tests/test_window.cpp -o build/tests/test_window `pkg-config --cflags --libs sdl2 SDL2_ttf`
	@echo "✓ Test built: build/tests/test_window"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_window

test-os-dev: stdlib/system/system.cpp stdlib/tests/test_os_dev.cpp
	@echo "Building OS development test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O2 -I. stdlib/system/system.cpp stdlib/tests/test_os_dev.cpp -o build/tests/test_os_dev -lpthread
	@echo "✓ Test built: build/tests/test_os_dev"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_os_dev

test-hpc: stdlib/hpc/simd.cpp stdlib/tests/test_hpc.cpp
	@echo "Building HPC/SIMD test..."
	@mkdir -p build/tests
	$(CXX) -std=c++20 -Wall -Wextra -O3 -march=native -I. stdlib/hpc/simd.cpp stdlib/tests/test_hpc.cpp -o build/tests/test_hpc -lpthread
	@echo "✓ Test built: build/tests/test_hpc"
	@echo ""
	@echo "Running tests..."
	@./build/tests/test_hpc

test-new-stdlib: test-datetime test-system test-graphics test-utils test-algorithms test-window test-os-dev test-hpc
	@echo ""
	@echo "================================================================================";
	@echo "All new stdlib tests passed! 🚀✅"
	@echo "================================================================================";
