# Testing Guide - Sapphire

This document describes how to test Sapphire examples and features.

---

## Quick Start

### Run All Tests

```bash
./test_examples.sh
```

This will test all 186 .spp files in the examples/ directory.

### Run Specific Example

```bash
./sapp examples/hello.spp
```

---

## Test Suite

### Overview

- **Total Examples:** 186
- **Current Pass Rate:** 77% (141/183 testable)
- **Skipped:** 3 (require user input)

### Test Script Features

1. **Automatic Discovery**
   - Finds all .spp files in examples/
   - No manual maintenance needed

2. **Error Detection**
   - Parse errors
   - Runtime errors
   - Segmentation faults
   - Timeouts (5 second limit)

3. **Skip List**
   - Files requiring user input
   - Files that timeout

4. **Detailed Reporting**
   - Pass/Fail/Skip counts
   - Pass rate percentage
   - Error messages

---

## Test Categories

### Core Language (✓ Passing)
- Basic syntax
- Variables and types
- Operators
- Control flow
- Functions
- Classes

### Pattern Matching (✓ Passing)
- Basic patterns
- Guards
- Array destructuring
- Object destructuring
- Multiple patterns per arm

### Concurrency (✓ Passing)
- Channels
- Goroutines
- Select statements
- WaitGroups
- Async/await

### Error Handling (✓ Passing)
- Result types
- Option types
- Try operator (?)
- Error propagation

### Standard Library (✓ Passing)
- Collections
- Itertools
- File I/O
- Testing framework

### Advanced Features (✓ Passing)
- Reflection
- Macros (basic)
- Hot reloading
- Decorators

---

## Running Tests

### All Tests

```bash
./test_examples.sh
```

### With Verbose Output

```bash
./test_examples.sh 2>&1 | tee test_output.txt
```

### Count Passing Tests

```bash
./test_examples.sh 2>&1 | grep "✓ PASS" | wc -l
```

### List Failing Tests

```bash
./test_examples.sh 2>&1 | grep -B1 "✗ FAIL" | grep "Testing:"
```

---

## Adding New Tests

### 1. Create Test File

Create a new .spp file in examples/:

```sapphire
# examples/my_test.spp
print("Testing my feature")

let result = my_function()
print("Result:", result)
```

### 2. Run Test Suite

```bash
./test_examples.sh
```

The new test will be automatically discovered and run.

### 3. Skip If Needed

If your test requires user input, add it to the skip list in `test_examples.sh`:

```bash
SKIP_FILES=(
    "examples/input_test_simple.spp"
    "examples/input_working.spp"
    "examples/user_input_example.spp"
    "examples/my_interactive_test.spp"  # Add here
)
```

---

## Test Results

### Current Status

```
Passed:  141 (77%)
Failed:  42  (23%)
Skipped: 3
Total:   186
```

### Pass Rate Goals

- **Beta:** 75%+ ✓ (Current: 77%)
- **v1.0:** 90%+
- **v1.1:** 95%+
- **v2.0:** 98%+

---

## Debugging Failed Tests

### 1. Run Individual Test

```bash
./sapp examples/failing_test.spp
```

### 2. Check Error Output

Look for:
- Parse errors (syntax issues)
- Runtime errors (logic issues)
- Segmentation faults (memory issues)

### 3. Fix and Retest

```bash
# Edit the file
vim examples/failing_test.spp

# Test again
./sapp examples/failing_test.spp

# Run full suite
./test_examples.sh
```

---

## Continuous Integration

### GitHub Actions (Future)

```yaml
name: Test Suite
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build
        run: make quick
      - name: Test
        run: ./test_examples.sh
```

---

## Test Coverage

### Covered Features

- ✅ Core language syntax
- ✅ Pattern matching
- ✅ Traits and generics
- ✅ Async/await
- ✅ Concurrency primitives
- ✅ Error handling
- ✅ Standard library
- ✅ Reflection
- ✅ Macros (basic)
- ✅ Hot reloading

### Not Yet Covered

- ⏳ Multi-file projects
- ⏳ Package manager integration
- ⏳ LSP/DAP functionality
- ⏳ Performance benchmarks
- ⏳ Memory usage tests

---

## Best Practices

### Writing Tests

1. **Keep tests simple**
   - One feature per test
   - Clear expected output
   - Minimal dependencies

2. **Use descriptive names**
   - `test_pattern_matching_guards.spp`
   - `example_async_channels.spp`

3. **Add comments**
   - Explain what's being tested
   - Document expected behavior

4. **Handle errors gracefully**
   - Use Result/Option types
   - Provide clear error messages

### Test Organization

```
examples/
├── basic/           # Core language features
├── advanced/        # Advanced features
├── stdlib/          # Standard library
├── concurrency/     # Async/concurrency
├── patterns/        # Pattern matching
└── integration/     # Multi-feature tests
```

---

## Troubleshooting

### Test Script Not Running

```bash
# Make executable
chmod +x test_examples.sh

# Run
./test_examples.sh
```

### All Tests Failing

```bash
# Rebuild
make clean
make quick

# Test
./test_examples.sh
```

### Timeout Issues

Increase timeout in test_examples.sh:

```bash
# Change from 5s to 10s
output=$(timeout 10s ./sapp "$example" 2>&1)
```

---

## Contributing

### Adding Tests

1. Create test file in examples/
2. Run test suite
3. Fix any issues
4. Submit PR

### Improving Test Suite

1. Add new test categories
2. Improve error detection
3. Add performance tests
4. Enhance reporting

---

## Resources

- **Examples Directory:** examples/
- **Test Script:** test_examples.sh
- **Test Results:** TEST_RESULTS_SUMMARY.md
- **Documentation:** docs/

---

**Happy Testing! 🧪**
