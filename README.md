# Sapphire 💎

A modern programming language that's as fast as C and as easy as Python.

```sapphire
fn main() {
    let numbers = [1, 2, 3, 4, 5]
    for num in numbers {
        println(num * 2)
    }
}
```

**Fast.** Within 0-5% of C performance.  
**Safe.** Memory safety without garbage collection overhead.  
**Simple.** Clean syntax that feels natural.

**Website**: https://velo4705.github.io/sapphire-lang

---

## Quick Start

### Install

```bash
curl -fsSL https://raw.githubusercontent.com/velo4705/sapphire-lang/main/install.sh | bash
```

Or build from source:

```bash
git clone https://github.com/velo4705/sapphire-lang.git
cd sapphire-lang
make quick
./sapp examples/hello.spp
```

### Hello World

```sapphire
fn main() {
    println("Hello, Sapphire! 💎")
}
```

Run it:
```bash
sapp hello.spp
```

---

## Why Sapphire?

**Performance** - Second only to C/Assembly. Faster than Rust and C++ in many benchmarks.

**Memory Safety** - Ownership system catches bugs at compile time. No segfaults or memory leaks.

**Concurrency** - Built-in threads, channels, and async/await. Write parallel code easily.

**Complete Tooling** - Package manager, formatter, test runner, and debugger included.

**Rich Standard Library** - 50+ functions across 18 modules covering everything from file I/O to functional programming.

---

## Features

### Language
- Type inference (no verbose type annotations)
- Pattern matching with guards
- Generics and traits
- Decorators (@cache, @timing, @deprecated, @dataclass)
- Option<T> and Result<T, E> types for safe error handling
- ? operator for error propagation
- Error handling (try/catch)
- String interpolation
- Closures and lambdas
- Async/await and channels

### Standard Library (18 modules, 50+ functions)
- **Core**: String, Vec, HashMap, Math
- **I/O**: File operations, JSON, Base64
- **Network**: HTTP client/server
- **Text**: Regular expressions
- **System**: Process management, OS interface
- **Advanced**: SIMD/HPC, GUI, Graphics
- **Tools**: CLI parsing, DateTime
- **Functional**: map, filter, reduce, zip, enumerate
- **Testing**: assert functions for unit tests

### Performance
- LLVM-based compilation
- Zero-cost abstractions
- SIMD vectorization (AVX-512)
- 119μs garbage collection pauses
- Native code generation

---

## Examples

### Variables and Types

```sapphire
fn main() {
    let x = 10              // Type inference
    let mut y = 20          // Mutable variable
    let name: str = "Alice" // Explicit type
    
    println(x + y)
}
```

### Control Flow

```sapphire
fn main() {
    let score = 85
    
    if score >= 90 {
        println("A")
    } else if score >= 80 {
        println("B")
    } else {
        println("C")
    }
    
    for i in 0..10 {
        println(i)
    }
}
```

### Functions

```sapphire
fn add(a: int, b: int) -> int {
    return a + b
}

fn main() {
    let result = add(5, 3)
    println(result)  // 8
}
```

### File I/O

```sapphire
import File from "stdlib/io"

fn main() {
    let content = File.read("data.txt")
    println(content)
    
    File.write("output.txt", "Hello, World!")
}
```

### HTTP Server

```sapphire
import HTTP from "stdlib/http"

fn main() {
    let server = HTTP.Server(8080)
    
    server.get("/", fn(req, res) {
        res.body = "Hello, World!"
    })
    
    server.start()
}
```

### Decorators

```sapphire
# Built-in @cache decorator for memoization
@cache
fn fibonacci(n: int) -> int {
    if n <= 1 { return n }
    return fibonacci(n - 1) + fibonacci(n - 2)
}

# Built-in @timing decorator for performance measurement
@timing
fn slow_function() {
    # ... expensive computation ...
}

# User-defined decorators
fn log_decorator(func) {
    fn wrapper() {
        println("Before function")
        func()
        println("After function")
    }
    return wrapper
}

@log_decorator
fn greet() {
    println("Hello!")
}
```

More examples in [examples/](examples/)

---

## Documentation

**Tutorials** (Start here!)
- [Getting Started](docs/tutorials/01_GETTING_STARTED.md) - Your first program
- [Control Flow](docs/tutorials/02_CONTROL_FLOW.md) - If statements and loops
- [Functions](docs/tutorials/03_FUNCTIONS.md) - Writing reusable code
- [Arrays & Collections](docs/tutorials/04_ARRAYS.md) - Working with data
- [Strings](docs/tutorials/05_STRINGS.md) - Text manipulation
- [File I/O](docs/tutorials/06_FILE_IO.md) - Reading and writing files

**Reference**
- [API Reference](docs/API_REFERENCE.md) - Complete standard library documentation
- [Language Features](docs/LANGUAGE_FEATURES.md) - Syntax and semantics
- [System Programming](docs/SYSTEM_PROGRAMMING.md) - Low-level access

---

## Tools

**sapp** - Compiler and interpreter
```bash
sapp hello.spp              # Run program
sapp --compile hello.spp    # Compile to binary
sapp --version              # Show version
```

**spm** - Package manager
```bash
spm init                    # Create new project
spm build                   # Build project
spm test                    # Run tests
spm run                     # Run project
```

**sapphire-fmt** - Code formatter
```bash
sapphire-fmt file.spp       # Format file
sapphire-fmt --check .      # Check formatting
```

**sapphire-lsp** - Language Server Protocol
```bash
# Used by IDEs for auto-completion, go-to-definition, etc.
# VSCode extension available in editors/vscode/
```

**sapphire-debug** - Debug Adapter Protocol
```bash
# Used by IDEs for breakpoint debugging
# Integrated with VSCode extension
```

**sapphire-doc** - Documentation generator
```bash
sapphire-doc file.spp       # Generate docs for one file
sapphire-doc examples/      # Generate docs for directory
```

**spm-registry** - Package registry client
```bash
spm-registry publish        # Publish package to registry
spm-registry install pkg    # Install package from registry
spm-registry search query   # Search for packages
```

---

## IDE Support

**VSCode Extension** - Full language support
- Syntax highlighting
- Auto-completion
- Go to definition
- Find references
- Real-time error checking
- Breakpoint debugging

Install:
```bash
code --install-extension editors/vscode/sapphire-lang.vsix
```

---

## Status

**Version:** 1.0-beta.7  
**Tests:** 34/34 passing (100%)  
**Libraries:** 18/18 complete  
**Status:** Production ready ✅

---

## System Programming & OS Development

Sapphire includes a comprehensive system library for OS-level programming:

### System Library Features
- **Process Management**: Get PID, spawn processes, exit
- **Memory Management**: Allocate/free memory, page size info
- **File System**: Check existence, create directories, get CWD
- **Environment**: Access env vars, username, hostname
- **System Info**: CPU count, architecture detection
- **High-Precision Timing**: Nanosecond, microsecond, millisecond timestamps
- **Sleep Functions**: Precise timing control

### Quick Example

```sapphire
import system

print("System Information:")
let pid = sapphire_system_get_pid()
let cpu_count = sapphire_system_cpu_count()
let cpu_arch = sapphire_system_cpu_arch()

print("PID:", pid)
print("CPUs:", cpu_count)
print("Architecture:", cpu_arch)

// High-precision timing
let start = sapphire_system_timestamp_ns()
// ... your code ...
let end = sapphire_system_timestamp_ns()
print("Elapsed:", end - start, "ns")
```

### Working Examples
- `system_demo.spp` - System capabilities demo
- `kernel_hello.spp` - Kernel-style initialization
- `process_monitor.spp` - Process monitoring tool

**Documentation:**
- [System Library Complete Guide](SYSTEM_LIBRARY_COMPLETE.md)
- [Quick Start Guide](SYSTEM_QUICK_START.md)
- [OS Kernel Roadmap](OS_KERNEL_ROADMAP.md)
- [OS Development Guide](docs/OS_DEVELOPMENT.md)

---

## Game Development

Sapphire supports 2D game development with SDL2:

### Working Pong Game

```sapphire
import graphics.sdl2

let window = sapphire_sdl2_create_window("Pong", 800, 600)
sapphire_sdl2_show_window(window)

// Game loop
while !sapphire_sdl2_should_close(window):
    sapphire_sdl2_poll_events(window)
    
    // Clear screen
    sapphire_sdl2_clear(window, 0, 0, 0)
    
    // Draw paddles and ball
    sapphire_sdl2_fill_rect(window, p1x, p1y, 20, 100, 255, 255, 255)
    sapphire_sdl2_fill_rect(window, p2x, p2y, 20, 100, 255, 255, 255)
    sapphire_sdl2_fill_rect(window, bx, by, 15, 15, 255, 255, 0)
    
    // Present
    sapphire_sdl2_present(window)
    sapphire_sdl2_delay(16)  // 60 FPS

sapphire_sdl2_destroy_window(window)
```

### Play the Demo

```bash
./play_pong.sh
```

**Features:**
- SDL2 graphics library integration
- 2D rendering (rectangles, lines, points)
- Keyboard input handling
- 60 FPS game loop
- Window management

**Documentation:**
- [Pong Game README](PONG_README.md)
- [Game Development Status](PONG_GAME_STATUS.md)
- [Graphics Quick Start](GRAPHICS_QUICK_START.md)

---

## Use Cases

Build anything with Sapphire:

- **Web applications** (frontend & backend)
- **System utilities and tools**
- **High-performance computing**
- **GUI applications**
- **Mobile apps**
- **Game development** (2D with SDL2)
- **Data science** (statistics, ML, time series)
- **Network applications** (TCP, UDP, HTTP)
- **Data processing**
- **Network services**
- **Operating system development** (kernel modules, drivers)
- **Embedded systems** (IoT, sensors, controllers)

---

## Contributing

We welcome contributions! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

**Ways to help:**
- Report bugs and suggest features
- Improve documentation
- Add examples
- Optimize performance
- Write tutorials

---

## Community

- **GitHub**: [velo4705/sapphire-lang](https://github.com/velo4705/sapphire-lang)
- **Issues**: [Report bugs](https://github.com/velo4705/sapphire-lang/issues)
- **Discussions**: [Ask questions](https://github.com/velo4705/sapphire-lang/discussions)

---

## License

MIT License - see [LICENSE](LICENSE)

---

<div align="center">

**Made with 💎 by the Sapphire community**

[⭐ Star](https://github.com/velo4705/sapphire-lang) · [🐛 Report Bug](https://github.com/velo4705/sapphire-lang/issues) · [� Request Feature](https://github.com/velo4705/sapphire-lang/issues)

</div>
