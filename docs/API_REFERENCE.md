# Sapphire API Reference

Complete reference for language features and standard library.

## Table of Contents

### Language Features (v1.1)
- [Pattern Matching](#pattern-matching) - Match expressions with guards
- [Traits](#traits) - Rust-style traits for polymorphism

### Core Libraries
- [DateTime](#datetime) - Date and time operations
- [System](#system) - Operating system interaction
- [Utils](#utils) - Utility functions (strings, random, hash)
- [Math](#math) - Mathematical functions

### Data Structures
- [Algorithms](#algorithms) - Sorting, searching, graph algorithms
- [Core Collections](#core-collections) - Vec, HashMap, String
- [Data Structures](#data-structures) - Advanced data structures

### I/O and Encoding
- [File I/O](#file-io) - File operations
- [JSON](#json) - JSON parsing and serialization
- [Base64](#base64) - Base64 encoding/decoding

### Networking and Text Processing
- [HTTP](#http) - HTTP client and server
- [Regex](#regex) - Regular expression matching

### Graphics and GUI
- [Graphics](#graphics) - 2D graphics primitives
- [GUI Window](#gui-window) - Window management

### Advanced
- [HPC/SIMD](#hpcsimd) - High-performance computing
- [CLI](#cli) - Command-line argument parsing

---

## Pattern Matching

**Status:** Core features implemented (v1.1)  
**See:** [Pattern Matching Guide](PATTERN_MATCHING.md)

### Syntax

```sapphire
match <value>:
    <pattern1> => <expression1>
    <pattern2> => <expression2>
    _ => <default>
```

### Example

```sapphire
fn classify(n):
    return match n:
        0 => "zero"
        1 => "one"
        2 => "two"
        _ => "other"

print(classify(0))  # "zero"
print(classify(5))  # "other"
```

### Features
- ✅ Literal patterns (0, "hello", true)
- ✅ Variable patterns (x, name)
- ✅ Wildcard pattern (_)
- ✅ Multiple arms
- 🟡 Guard clauses (x if x > 0) - in progress
- 🟡 Array destructuring ([a, b, c]) - planned
- 🟡 Object destructuring ({x, y}) - planned

---

## Traits

**Status:** Core features implemented (v1.1)  
**See:** [Traits Guide](TRAITS.md)

### Defining Traits

```sapphire
trait Drawable:
    fn draw(self) -> str
    fn area(self) -> float
```

### Implementing Traits

```sapphire
class Circle:
    fn __init__(self, r):
        self.radius = r

impl Drawable for Circle:
    fn draw(self) -> str:
        return "Circle"
    fn area(self) -> float:
        return 3.14159 * self.radius * self.radius
```

### Using Trait Methods

```sapphire
let c = Circle(5.0)
print(c.draw())   # Calls trait method
print(c.area())   # Calls trait method
```

### Features
- ✅ Trait definitions
- ✅ Impl blocks (standalone)
- ✅ Trait method dispatch
- ✅ Multiple traits per type
- ✅ Multiple types per trait
- 🟡 Trait bounds in generics - planned
- 🟡 Where clauses - planned
- 🟡 Associated types - planned
- 🟡 Default methods - planned

---

## DateTime

**Header:** `stdlib/datetime/datetime.h`  
**Import:** `import DateTime from "stdlib/datetime"`

High-precision date and time operations.

### Classes

#### Instant
Represents a point in time with millisecond precision.

```cpp
// C++ API
Instant* instant = Instant::now();
int64_t ms = instant->timestamp_ms();
int64_t sec = instant->timestamp_sec();
```

```sapphire
// Sapphire API
let now = DateTime.now()
let timestamp = now.timestamp_ms()
```

**Methods:**
- `now()` - Get current time
- `timestamp_ms()` - Get milliseconds since epoch
- `timestamp_sec()` - Get seconds since epoch

#### Date
Represents a calendar date.

```cpp
Date* date = new Date(2026, 3, 6);
int year = date->year();
int month = date->month();
int day = date->day();
```

**Methods:**
- `year()` - Get year
- `month()` - Get month (1-12)
- `day()` - Get day (1-31)

#### Time
Represents time of day.

```cpp
Time* time = new Time(14, 30, 45);
int hour = time->hour();
int minute = time->minute();
int second = time->second();
```

**Methods:**
- `hour()` - Get hour (0-23)
- `minute()` - Get minute (0-59)
- `second()` - Get second (0-59)

### C API

```c
void* sapphire_instant_now();
int64_t sapphire_instant_timestamp_ms(void* instant);
void* sapphire_date_create(int year, int month, int day);
void* sapphire_time_create(int hour, int minute, int second);
```

---

## System

**Header:** `stdlib/system/system.h`  
**Import:** `import System from "stdlib/system"`

Operating system interaction and system calls.

### Process Management

```cpp
int pid = System::get_pid();
int ppid = System::get_parent_pid();
int child = System::spawn_process("/bin/ls", {"-l"});
int status = System::wait_process(child);
System::exit_process(0);
```

**Functions:**
- `get_pid()` - Get current process ID
- `get_parent_pid()` - Get parent process ID
- `spawn_process(cmd, args)` - Spawn new process
- `wait_process(pid)` - Wait for process to complete
- `exit_process(code)` - Exit with status code

### Memory Management

```cpp
void* ptr = System::allocate_memory(1024);
System::free_memory(ptr);
size_t page_size = System::get_page_size();
size_t total = System::get_total_memory();
size_t available = System::get_available_memory();
```

**Functions:**
- `allocate_memory(size)` - Allocate memory
- `free_memory(ptr)` - Free memory
- `reallocate_memory(ptr, size)` - Reallocate memory
- `get_page_size()` - Get system page size
- `get_total_memory()` - Get total RAM
- `get_available_memory()` - Get available RAM

### File System

```cpp
bool exists = System::file_exists("file.txt");
bool is_dir = System::directory_exists("folder");
System::create_directory("new_folder");
System::remove_file("file.txt");
std::vector<std::string> files = System::list_directory(".");
std::string cwd = System::get_current_directory();
System::change_directory("/tmp");
```

**Functions:**
- `file_exists(path)` - Check if file exists
- `directory_exists(path)` - Check if directory exists
- `create_directory(path)` - Create directory
- `remove_file(path)` - Delete file
- `remove_directory(path)` - Delete directory
- `list_directory(path)` - List directory contents
- `get_current_directory()` - Get current working directory
- `change_directory(path)` - Change directory

### Environment

```cpp
std::string value = System::get_env("PATH");
System::set_env("MY_VAR", "value");
std::string user = System::get_username();
std::string host = System::get_hostname();
std::string os = System::get_os_name();
```

**Functions:**
- `get_env(name)` - Get environment variable
- `set_env(name, value)` - Set environment variable
- `get_username()` - Get current username
- `get_hostname()` - Get hostname
- `get_os_name()` - Get OS name (Linux/macOS/Windows)

### System Information

```cpp
int cpus = System::get_cpu_count();
std::string arch = System::get_cpu_arch();
uint64_t uptime = System::get_uptime();
double load = System::get_load_average();
```

**Functions:**
- `get_cpu_count()` - Get number of CPUs
- `get_cpu_arch()` - Get CPU architecture (x86_64, aarch64, etc.)
- `get_uptime()` - Get system uptime in seconds
- `get_load_average()` - Get system load average

### High-Precision Timing

```cpp
uint64_t ns = System::get_timestamp_ns();
uint64_t us = System::get_timestamp_us();
uint64_t ms = System::get_timestamp_ms();
System::sleep_ms(1000);
System::sleep_us(1000);
```

**Functions:**
- `get_timestamp_ns()` - Get nanosecond timestamp
- `get_timestamp_us()` - Get microsecond timestamp
- `get_timestamp_ms()` - Get millisecond timestamp
- `sleep_ms(ms)` - Sleep for milliseconds
- `sleep_us(us)` - Sleep for microseconds

### Kernel Interface (OS Development)

```cpp
int result = Kernel::syscall(39, nullptr, nullptr);  // SYS_getpid
void* addr = Kernel::map_memory(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS);
Kernel::unmap_memory(addr, 4096);
uint8_t value = Kernel::inb(0x3F8);
Kernel::outb(0x3F8, 0x41);
Kernel::enable_interrupts();
Kernel::disable_interrupts();
```

**Functions:**
- `syscall(number, arg1, arg2)` - Direct system call
- `map_memory(addr, length, prot, flags)` - Memory mapping (mmap)
- `unmap_memory(addr, length)` - Unmap memory (munmap)
- `inb(port)` - Read byte from I/O port
- `outb(port, value)` - Write byte to I/O port
- `inw(port)` - Read word from I/O port
- `outw(port, value)` - Write word to I/O port
- `inl(port)` - Read dword from I/O port
- `outl(port, value)` - Write dword to I/O port
- `enable_interrupts()` - Enable interrupts (STI)
- `disable_interrupts()` - Disable interrupts (CLI)

---

## Utils

**Header:** `stdlib/utils/utils.h`  
**Import:** `import Utils from "stdlib/utils"`

Utility functions for common operations.

### String Operations

```cpp
std::string upper = Utils::to_upper("hello");
std::string lower = Utils::to_lower("HELLO");
std::string trimmed = Utils::trim("  hello  ");
std::vector<std::string> parts = Utils::split("a,b,c", ",");
std::string joined = Utils::join({"a", "b", "c"}, ",");
std::string replaced = Utils::replace("hello world", "world", "there");
bool starts = Utils::starts_with("hello", "hel");
bool ends = Utils::ends_with("hello", "lo");
bool contains = Utils::contains("hello", "ell");
```

**Functions:**
- `to_upper(str)` - Convert to uppercase
- `to_lower(str)` - Convert to lowercase
- `trim(str)` - Remove whitespace from both ends
- `ltrim(str)` - Remove whitespace from left
- `rtrim(str)` - Remove whitespace from right
- `split(str, delimiter)` - Split string
- `join(strings, delimiter)` - Join strings
- `replace(str, old, new)` - Replace substring
- `starts_with(str, prefix)` - Check if starts with
- `ends_with(str, suffix)` - Check if ends with
- `contains(str, substring)` - Check if contains

### Type Conversion

```cpp
std::string s = Utils::to_string(42);
int i = Utils::to_int("42");
double d = Utils::to_double("3.14");
bool b = Utils::to_bool("true");
```

**Functions:**
- `to_string(value)` - Convert to string
- `to_int(str)` - Convert to integer
- `to_double(str)` - Convert to double
- `to_bool(str)` - Convert to boolean

### Random Numbers

```cpp
int random_int = Utils::random_int(1, 100);
double random_double = Utils::random_double(0.0, 1.0);
Utils::seed_random(12345);
```

**Functions:**
- `random_int(min, max)` - Random integer in range
- `random_double(min, max)` - Random double in range
- `seed_random(seed)` - Seed random number generator

### Hash Functions

```cpp
uint64_t hash1 = Utils::hash_string("hello");
uint64_t hash2 = Utils::hash_int(42);
```

**Functions:**
- `hash_string(str)` - Hash string
- `hash_int(value)` - Hash integer

---

## Algorithms

**Header:** `stdlib/algorithms/algorithms.h`  
**Import:** `import Algorithms from "stdlib/algorithms"`

Sorting, searching, and graph algorithms.

### Sorting

```cpp
std::vector<int> arr = {5, 2, 8, 1, 9};
Algorithms::quick_sort(arr.data(), arr.size());
Algorithms::merge_sort(arr.data(), arr.size());
Algorithms::heap_sort(arr.data(), arr.size());
```

**Functions:**
- `quick_sort(arr, size)` - Quick sort O(n log n) average
- `merge_sort(arr, size)` - Merge sort O(n log n)
- `heap_sort(arr, size)` - Heap sort O(n log n)

### Searching

```cpp
int index = Algorithms::binary_search(arr.data(), arr.size(), 5);
int linear = Algorithms::linear_search(arr.data(), arr.size(), 5);
```

**Functions:**
- `binary_search(arr, size, target)` - Binary search O(log n)
- `linear_search(arr, size, target)` - Linear search O(n)

### Graph Algorithms

```cpp
// BFS
std::vector<int> bfs_result = Algorithms::bfs(graph, start);

// DFS
std::vector<int> dfs_result = Algorithms::dfs(graph, start);

// Dijkstra's shortest path
std::vector<int> distances = Algorithms::dijkstra(graph, start);
```

**Functions:**
- `bfs(graph, start)` - Breadth-first search O(V + E)
- `dfs(graph, start)` - Depth-first search O(V + E)
- `dijkstra(graph, start)` - Shortest path O((V + E) log V)

### Dynamic Programming

```cpp
int64_t fib = Algorithms::fibonacci(10);
int lcs_len = Algorithms::lcs("ABCD", "ACBD");
int knapsack_val = Algorithms::knapsack(weights, values, capacity);
```

**Functions:**
- `fibonacci(n)` - Fibonacci number O(n)
- `lcs(str1, str2)` - Longest common subsequence O(m × n)
- `knapsack(weights, values, capacity)` - 0/1 knapsack O(n × W)

### String Algorithms

```cpp
int index = Algorithms::kmp_search("hello world", "world");
int rabin = Algorithms::rabin_karp("hello world", "world");
int distance = Algorithms::edit_distance("kitten", "sitting");
```

**Functions:**
- `kmp_search(text, pattern)` - KMP pattern matching O(n + m)
- `rabin_karp(text, pattern)` - Rabin-Karp search O(n + m)
- `edit_distance(str1, str2)` - Edit distance O(m × n)

---

## HTTP

**Header:** `stdlib/http/http.h`  
**Import:** `import HTTP from "stdlib/http"`

HTTP client and server for web communication.

### HTTP Client

```cpp
// Simple GET request
HttpResponse response = HttpClient::get("http://example.com/api/data");
std::cout << "Status: " << response.status_code << std::endl;
std::cout << "Body: " << response.body << std::endl;

// POST request with body
HttpResponse post_response = HttpClient::post(
    "http://example.com/api/users",
    "{\"name\":\"Alice\",\"age\":25}"
);

// PUT and DELETE
HttpClient::put("http://example.com/api/users/1", "{\"age\":26}");
HttpClient::delete_request("http://example.com/api/users/1");

// Set default headers
HttpClient::set_header("Authorization", "Bearer token123");
HttpClient::set_timeout(60);
```

**Client Methods:**
- `get(url)` - HTTP GET request
- `post(url, body)` - HTTP POST request
- `put(url, body)` - HTTP PUT request
- `delete_request(url)` - HTTP DELETE request
- `set_header(key, value)` - Set default header
- `set_timeout(seconds)` - Set request timeout

### HTTP Server

```cpp
HttpServer server(8080);

// Define routes
server.get("/", [](const HttpRequest& req, HttpResponse& res) {
    res.body = "Hello, World!";
});

server.post("/api/users", [](const HttpRequest& req, HttpResponse& res) {
    // Handle POST request
    res.status_code = 201;
    res.body = "{\"id\":1,\"created\":true}";
});

// Serve static files
server.serve_static("/static", "./public");

// Start server (blocking)
server.start();
```

**Server Methods:**
- `HttpServer(port)` - Create server on port
- `get(path, handler)` - Register GET route
- `post(path, handler)` - Register POST route
- `put(path, handler)` - Register PUT route
- `delete_route(path, handler)` - Register DELETE route
- `serve_static(path, directory)` - Serve static files
- `start()` - Start server (blocking)
- `stop()` - Stop server

### URL Utilities

```cpp
// URL encoding/decoding
std::string encoded = URL::encode("hello world");  // "hello%20world"
std::string decoded = URL::decode("hello%20world");  // "hello world"

// Query string parsing
std::map<std::string, std::string> params = URL::parse_query("name=Alice&age=25");
// params["name"] = "Alice", params["age"] = "25"

// Build query string
std::map<std::string, std::string> data = {{"name", "Bob"}, {"age", "30"}};
std::string query = URL::build_query(data);  // "name=Bob&age=30"
```

**URL Methods:**
- `encode(str)` - URL encode string
- `decode(str)` - URL decode string
- `parse_query(query)` - Parse query string to map
- `build_query(params)` - Build query string from map

---

## Regex

**Header:** `stdlib/regex/regex.h`  
**Import:** `import Regex from "stdlib/regex"`

Regular expression pattern matching and text manipulation.

### Basic Matching

```cpp
// Check if entire string matches pattern
bool matches = Regex::matches("\\d+", "12345");  // true
bool no_match = Regex::matches("\\d+", "abc");   // false

// Search for pattern in string
bool found = Regex::search("\\d+", "abc123def");  // true
```

### Finding Matches

```cpp
// Find first match
Match match = Regex::find("\\d+", "abc123def456");
std::cout << "Found: " << match.text << std::endl;  // "123"
std::cout << "Position: " << match.position << std::endl;  // 3

// Find all matches
std::vector<Match> matches = Regex::find_all("\\d+", "abc123def456ghi789");
for (const auto& m : matches) {
    std::cout << m.text << " at " << m.position << std::endl;
}
// Output:
// 123 at 3
// 456 at 9
// 789 at 15
```

### Capture Groups

```cpp
Match match = Regex::find("(\\w+)@(\\w+\\.\\w+)", "user@example.com");
std::cout << "Full match: " << match.text << std::endl;  // "user@example.com"
std::cout << "Group 1: " << match.groups[0] << std::endl;  // "user"
std::cout << "Group 2: " << match.groups[1] << std::endl;  // "example.com"
```

### Replacement

```cpp
// Replace first occurrence
std::string result = Regex::replace("\\d+", "abc123def", "XXX");
// result = "abcXXXdef"

// Replace all occurrences
std::string result_all = Regex::replace_all("\\d+", "abc123def456", "XXX");
// result_all = "abcXXXdefXXX"
```

### Splitting

```cpp
// Split by pattern
std::vector<std::string> parts = Regex::split("[,;]", "a,b;c,d");
// parts = ["a", "b", "c", "d"]

// Split by whitespace
std::vector<std::string> words = Regex::split("\\s+", "hello  world\tfoo");
// words = ["hello", "world", "foo"]
```

### Regex Object

```cpp
// Create reusable regex object
Regex email_regex("\\w+@\\w+\\.\\w+");

// Use multiple times
bool valid1 = email_regex.matches("user@example.com");  // true
bool valid2 = email_regex.matches("invalid.email");     // false

// Case-insensitive matching
Regex case_insensitive("hello", true);
bool matches = case_insensitive.matches("HELLO");  // true
```

**Regex Methods:**
- `Regex(pattern)` - Create regex from pattern
- `Regex(pattern, case_insensitive)` - Create with options
- `matches(text)` - Check if entire string matches
- `search(text)` - Check if pattern found in string
- `find(text)` - Find first match
- `find_all(text)` - Find all matches
- `replace(text, replacement)` - Replace first match
- `replace_all(text, replacement)` - Replace all matches
- `split(text)` - Split by pattern

**Static Methods:**
- `matches(pattern, text)` - Quick match check
- `search(pattern, text)` - Quick search
- `find(pattern, text)` - Quick find
- `find_all(pattern, text)` - Quick find all
- `replace(pattern, text, replacement)` - Quick replace
- `replace_all(pattern, text, replacement)` - Quick replace all
- `split(pattern, text)` - Quick split
- `is_valid(pattern)` - Check if pattern is valid
- `escape(str)` - Escape special regex characters

### Common Patterns

```cpp
// Email validation
Regex email("^[\\w.-]+@[\\w.-]+\\.\\w+$");

// Phone number (US format)
Regex phone("^\\(?\\d{3}\\)?[-.\\s]?\\d{3}[-.\\s]?\\d{4}$");

// URL
Regex url("^https?://[\\w.-]+\\.[\\w]{2,}(/.*)?$");

// IPv4 address
Regex ipv4("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$");

// Date (YYYY-MM-DD)
Regex date("^\\d{4}-\\d{2}-\\d{2}$");

// Hexadecimal color
Regex color("^#[0-9A-Fa-f]{6}$");
```

---

## Graphics

**Header:** `stdlib/graphics/graphics.h`  
**Import:** `import Graphics from "stdlib/graphics"`

2D graphics primitives and image export.

### Canvas

```cpp
Canvas* canvas = new Canvas(800, 600);
canvas->set_pixel(100, 100, Color::RED);
canvas->draw_line(0, 0, 100, 100, Color::BLUE);
canvas->draw_rect(50, 50, 100, 100, Color::GREEN);
canvas->fill_rect(200, 200, 50, 50, Color::YELLOW);
canvas->draw_circle(400, 300, 50, Color::MAGENTA);
canvas->export_ppm("output.ppm");
```

**Methods:**
- `Canvas(width, height)` - Create canvas
- `set_pixel(x, y, color)` - Set pixel color
- `draw_line(x1, y1, x2, y2, color)` - Draw line
- `draw_rect(x, y, w, h, color)` - Draw rectangle outline
- `fill_rect(x, y, w, h, color)` - Draw filled rectangle
- `draw_circle(x, y, radius, color)` - Draw circle outline
- `fill_circle(x, y, radius, color)` - Draw filled circle
- `export_ppm(filename)` - Export as PPM image

### Colors

```cpp
Color red = Color::RED;
Color custom = Color(255, 128, 0, 255);  // RGBA
```

**Predefined Colors:**
- `RED`, `GREEN`, `BLUE`, `YELLOW`, `MAGENTA`, `CYAN`, `WHITE`, `BLACK`

---

## GUI Window

**Header:** `stdlib/gui/window.h`  
**Import:** `import Window from "stdlib/gui"`

Cross-platform window management with SDL2.

### Window Class

```cpp
Window window("My App", 800, 600);
window.show();

while (window.is_open()) {
    window.poll_events();
    
    window.clear(20, 30, 50);  // Dark blue background
    window.draw_text("Hello, World!", 100, 100, 48, 255, 255, 255);
    window.fill_rect(50, 50, 100, 100, 255, 0, 0);
    window.present();
}
```

**Methods:**
- `Window(title, width, height)` - Create window
- `show()` - Show window
- `hide()` - Hide window
- `close()` - Close window
- `is_open()` - Check if window is open
- `poll_events()` - Process events
- `clear(r, g, b)` - Clear screen with color
- `present()` - Present frame
- `draw_text(text, x, y, size, r, g, b)` - Draw text
- `draw_rect(x, y, w, h, r, g, b)` - Draw rectangle outline
- `fill_rect(x, y, w, h, r, g, b)` - Draw filled rectangle
- `set_title(title)` - Set window title

---

## HPC/SIMD

**Header:** `stdlib/hpc/simd.h`  
**Import:** `import SIMD from "stdlib/hpc"`

High-performance computing with SIMD vectorization.

### Vector Types

```cpp
SIMD::Vec4f a(1.0f, 2.0f, 3.0f, 4.0f);
SIMD::Vec4f b(5.0f, 6.0f, 7.0f, 8.0f);
SIMD::Vec4f c = a + b;
float dot = a.dot(b);
float len = a.length();
```

**Vec4f Methods:**
- `operator+`, `operator-`, `operator*`, `operator/` - Arithmetic
- `dot(other)` - Dot product
- `length()` - Vector length
- `normalize()` - Normalize vector

### Array Operations

```cpp
float a[1000], b[1000], result[1000];
SIMD::add_arrays_f32(a, b, result, 1000);
SIMD::mul_arrays_f32(a, b, result, 1000);
float dot = SIMD::dot_product_f32(a, b, 1000);
```

**Functions:**
- `add_arrays_f32(a, b, result, size)` - Add arrays (AVX)
- `mul_arrays_f32(a, b, result, size)` - Multiply arrays (AVX)
- `dot_product_f32(a, b, size)` - Dot product (AVX)
- `matrix_multiply_f32(a, b, result, n)` - Matrix multiply

### Reduction Operations

```cpp
float sum = SIMD::sum_f32(arr, size);
float max_val = SIMD::max_f32(arr, size);
float min_val = SIMD::min_f32(arr, size);
```

**Functions:**
- `sum_f32(arr, size)` - Sum array elements
- `max_f32(arr, size)` - Find maximum
- `min_f32(arr, size)` - Find minimum

### Parallel Operations

```cpp
SIMD::parallel_add_f32(a, b, result, size, 4);  // 4 threads
SIMD::parallel_mul_f32(a, b, result, size, 4);
```

**Functions:**
- `parallel_add_f32(a, b, result, size, threads)` - Parallel add
- `parallel_mul_f32(a, b, result, size, threads)` - Parallel multiply

### Memory Operations

```cpp
void* ptr = Memory::aligned_alloc(64, 1024);
Memory::prefetch(ptr);
Memory::memory_fence();
Memory::aligned_free(ptr);
```

**Functions:**
- `aligned_alloc(alignment, size)` - Aligned allocation
- `aligned_free(ptr)` - Free aligned memory
- `prefetch(addr)` - Prefetch data
- `memory_fence()` - Memory barrier

---

## JSON

**Header:** `stdlib/json/json.h`  
**Import:** `import JSON from "stdlib/json"`

JSON parsing and serialization.

```cpp
// Parse JSON
std::string json_str = "{\"name\":\"Alice\",\"age\":25}";
JSON::Value* value = JSON::parse(json_str);

// Access values
std::string name = value->get("name")->as_string();
int age = value->get("age")->as_int();

// Create JSON
JSON::Value* obj = JSON::object();
obj->set("name", JSON::string("Bob"));
obj->set("age", JSON::number(30));

// Serialize
std::string output = JSON::stringify(obj);
```

---

## Math

**Header:** `stdlib/math/math.h`  
**Import:** `import Math from "stdlib/math"`

Mathematical functions.

```cpp
double result = Math::sqrt(16.0);
double power = Math::pow(2.0, 3.0);
double sine = Math::sin(3.14159 / 2);
double cosine = Math::cos(0.0);
double absolute = Math::abs(-5.5);
```

**Functions:**
- `sqrt(x)` - Square root
- `pow(x, y)` - Power
- `sin(x)`, `cos(x)`, `tan(x)` - Trigonometric
- `abs(x)` - Absolute value
- `floor(x)`, `ceil(x)`, `round(x)` - Rounding

---

## Usage Examples

### Example 1: File Processing with Utils

```cpp
#include "stdlib/io/file.h"
#include "stdlib/utils/utils.h"

int main() {
    std::string content = File::read("data.txt");
    std::vector<std::string> lines = Utils::split(content, "\n");
    
    for (const auto& line : lines) {
        std::string trimmed = Utils::trim(line);
        if (Utils::starts_with(trimmed, "#")) continue;
        std::cout << trimmed << std::endl;
    }
    
    return 0;
}
```

### Example 2: High-Performance Computing

```cpp
#include "stdlib/hpc/simd.h"

int main() {
    const size_t size = 1000000;
    float* a = new float[size];
    float* b = new float[size];
    float* result = new float[size];
    
    // Initialize arrays
    for (size_t i = 0; i < size; i++) {
        a[i] = i * 1.0f;
        b[i] = 2.0f;
    }
    
    // SIMD operations (8x faster with AVX)
    SIMD::add_arrays_f32(a, b, result, size);
    float dot = SIMD::dot_product_f32(a, b, size);
    
    delete[] a;
    delete[] b;
    delete[] result;
    
    return 0;
}
```

### Example 3: GUI Application

```cpp
#include "stdlib/gui/window.h"

int main() {
    Window window("My App", 800, 600);
    window.show();
    
    while (window.is_open()) {
        window.poll_events();
        
        window.clear(0, 0, 0);
        window.draw_text("Hello, Sapphire!", 250, 250, 48, 255, 255, 255);
        window.fill_rect(100, 100, 50, 50, 255, 0, 0);
        window.present();
    }
    
    return 0;
}
```

---

## See Also

- [Tutorials](tutorials/README.md) - Step-by-step learning
- [Language Features](LANGUAGE_FEATURES.md) - Language syntax
- [Getting Started](GETTING_STARTED.md) - Quick start guide
- [Examples](../examples/) - Example programs
