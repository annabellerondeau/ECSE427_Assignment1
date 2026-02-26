#!/bin/bash

# Check argument
if [ -z "$1" ]; then
    echo "Usage: ./run_tests.sh <TESTTYPE>"
    echo "Example: ./run_tests.sh FCFS"
    exit 1
fi

TYPE=$(echo "$1" | tr '[:lower:]' '[:upper:]')

TEST_DIR="test-cases-A2"
SHELL_PATH="../src/mysh"

if [ ! -f "$SHELL_PATH" ]; then
    echo "Error: mysh not found. Run make first."
    exit 1
fi

echo "Running $TYPE tests..."
echo "-----------------------------"

for f in $TEST_DIR/T_${TYPE}*.txt; do
    # Skip result files
    if [[ "$f" == *_result.txt ]]; then
        continue
    fi

    OUTPUT="tmp_out.txt"
    EXPECTED="${f%.txt}_result.txt"

    $SHELL_PATH < "$f" > "$OUTPUT"

    if diff -q "$OUTPUT" "$EXPECTED" > /dev/null; then
        echo "✅ $(basename $f) PASS"
    else
        echo "❌ $(basename $f) FAIL"
        echo "   Differences:"
        diff "$OUTPUT" "$EXPECTED"
    fi
done

rm -f tmp_out.txt