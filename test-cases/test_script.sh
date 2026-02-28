#!/bin/bash

# Check argument
if [ -z "$1" ]; then
    echo "Usage: ./run_tests.sh <TESTTYPE>"
    echo "Example: ./run_tests.sh FCFS"
    exit 1
fi

TYPE=$(echo "$1" | tr '[:lower:]' '[:upper:]')

SHELL_PATH="../src/mysh"

if [ ! -f "$SHELL_PATH" ]; then
    echo "Error: mysh not found. Run make first."
    exit 1
fi

echo "Running $TYPE tests..."
echo "-----------------------------"

for f in T_${TYPE}*.txt; do
    # Skip result files
    if [[ "$f" == *_result.txt ]]; then
        continue
    fi

    OUTPUT="tmp_out.txt"
    EXPECTED="${f%.txt}_result.txt"
    EXPECTED_CLEAN="tmp_expected.txt"

    # Remove 1 line from your shell output
    $SHELL_PATH < "$f" | tail -n +2 > "$OUTPUT"

    # Remove 2 lines from gold output (banner + blank line)
    tail -n +3 "$EXPECTED" > "$EXPECTED_CLEAN"

    if diff -q "$OUTPUT" "$EXPECTED_CLEAN" > /dev/null; then
        echo "✅ $(basename $f) PASS"
    else
        echo "❌ $(basename $f) FAIL"
        echo "   Differences:"
        diff "$OUTPUT" "$EXPECTED_CLEAN"
    fi
done

rm -f tmp_out.txt tmp_expected.txt