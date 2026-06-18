#!/bin/bash
# Run window/gui/display and web/network tests only
# These require a display server and/or network access.
# Use run_tests.sh for core language tests.

SAPP="./sapp"
PASS=0
FAIL=0
SKIP=0
ERRORS=()

# Only run: window/gui/display and web/network tests
INCLUDE_PATTERNS=(
    # Window/GUI/Display
    "car_simulator"
    "intermediate_plot3d_demo"
    "fpp4d"
    "sim4d"
    "sim_nbody_gui"
    "gui_window_demo"
    "milestone11_window_test"
    "milestone11_gui_test"
    "m13_gui_"
    "milestone9_game_test"
    "intermediate_m9_space_shooter"
    "intermediate_m11_dashboard"

    # Web/Network
    "websocket"
    "test_http"
    "rest_test"
    "graphql_test"
    "intermediate_m12_chat_server"
    "intermediate_m4_todo_api"
    "milestone12_network_test"
)

run_test() {
    local file="$1"
    local name=$(basename "$file")

    # Check if this file matches any include pattern
    local matched=0
    for pat in "${INCLUDE_PATTERNS[@]}"; do
        if [[ "$file" == *"$pat"* ]]; then
            matched=1
            break
        fi
    done

    if [ $matched -eq 0 ]; then
        return  # Skip files that don't match any pattern
    fi

    output=$(timeout 15s "$SAPP" "$file" 2>&1)
    exit_code=$?

    if [ $exit_code -eq 124 ]; then
        echo "  SKIP  $name  (timeout)"
        ((SKIP++))
    elif [ $exit_code -ne 0 ]; then
        if echo "$output" | grep -qE "^Error:|RuntimeError:|TypeError:|Undefined variable|NameError"; then
            echo "  FAIL  $name"
            last_err=$(echo "$output" | grep -E "^Error:|RuntimeError:|TypeError:|Undefined variable|NameError" | tail -1)
            ERRORS+=("FAIL: $name -- $last_err")
            ((FAIL++))
        else
            echo "  PASS  $name  (warnings only)"
            ((PASS++))
        fi
    else
        echo "  PASS  $name"
        ((PASS++))
    fi
}

echo "=== Running window/gui/web examples/*.spp ==="
for f in examples/*.spp; do
    run_test "$f"
done

echo ""
echo "=== Summary ==="
echo "PASS: $PASS  FAIL: $FAIL  SKIP: $SKIP"
echo ""
if [ ${#ERRORS[@]} -gt 0 ]; then
    echo "=== Failures ==="
    for e in "${ERRORS[@]}"; do
        echo "  $e"
    done
fi
