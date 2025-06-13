#!/bin/bash

CLIENT_SIZES=(10 50 100 200 300 400 500 600 700 800 900 1000 1050 1100 1200)
DURATION=100
SCRIPT="stress_test_logger_pbf.py"

for SIZE in "${CLIENT_SIZES[@]}"; do
    echo "=== Running stress test with $SIZE clients ==="
    python3 "$SCRIPT" --clients "$SIZE" --duration "$DURATION"

    echo "=== Completed $SIZE clients ==="
    echo "=== Waiting 10 seconds before next run... ==="
    sleep 10
done

echo "=== All stress tests complete ==="
