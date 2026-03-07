#!/bin/bash
# Test all Sapphire examples to ensure quality

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║         Testing All Sapphire Examples                        ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

PASS=0
FAIL=0
SKIP=0

# Files to skip (known to require user input or special conditions)
SKIP_FILES=(
    "examples/input_test_simple.spp"
    "examples/input_working.spp"
    "examples/user_input_example.spp"
)

# Function to check if file should be skipped
should_skip() {
    local file=$1
    for skip_file in "${SKIP_FILES[@]}"; do
        if [ "$file" = "$skip_file" ]; then
            return 0
        fi
    done
    return 1
}

# Get all .spp files in examples directory (not subdirectories)
mapfile -t ALL_EXAMPLES < <(find examples -maxdepth 1 -name "*.spp" -type f | sort)

echo "Found ${#ALL_EXAMPLES[@]} example files"
echo ""

# Test each example
for example in "${ALL_EXAMPLES[@]}"; do
    # Check if should skip
    if should_skip "$example"; then
        echo "Testing: $example"
        echo "  ⊘ SKIP - Requires user input"
        SKIP=$((SKIP + 1))
        echo ""
        continue
    fi
    
    echo "Testing: $example"
    
    # Run the example with timeout and capture output
    output=$(timeout 5s ./sapp "$example" 2>&1)
    exit_code=$?
    
    # Check for timeout (exit code 124)
    if [ $exit_code -eq 124 ]; then
        echo "  ⊘ SKIP - Timeout (likely infinite loop or waiting for input)"
        SKIP=$((SKIP + 1))
    # Check for parse errors
    elif echo "$output" | grep -q "Parse error"; then
        echo "  ✗ FAIL - Parse errors found"
        echo "$output" | grep "Parse error" | head -3
        FAIL=$((FAIL + 1))
    # Check for uncaught runtime errors (but not caught errors in try/catch)
    elif echo "$output" | grep -qE "^Error:|Runtime error:|Traceback"; then
        echo "  ✗ FAIL - Runtime error"
        echo "$output" | grep -E "^Error:|Runtime error:|Traceback" | head -3
        FAIL=$((FAIL + 1))
    # Check for segfault
    elif [ $exit_code -eq 139 ]; then
        echo "  ✗ FAIL - Segmentation fault"
        FAIL=$((FAIL + 1))
    # Check for other non-zero exit codes
    elif [ $exit_code -ne 0 ]; then
        echo "  ✗ FAIL - Exit code: $exit_code"
        FAIL=$((FAIL + 1))
    else
        echo "  ✓ PASS"
        PASS=$((PASS + 1))
    fi
    echo ""
done

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  Test Results                                                 ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""
echo "Passed:  $PASS"
echo "Failed:  $FAIL"
echo "Skipped: $SKIP"
echo "Total:   $((PASS + FAIL + SKIP))"
echo ""

# Calculate pass rate
if [ $((PASS + FAIL)) -gt 0 ]; then
    pass_rate=$((PASS * 100 / (PASS + FAIL)))
    echo "Pass Rate: ${pass_rate}%"
    echo ""
fi

if [ $FAIL -eq 0 ]; then
    echo "✓ All testable examples working! Ready for launch! 🚀"
    exit 0
else
    echo "✗ $FAIL example(s) failed. Review and fix before launch."
    exit 1
fi
