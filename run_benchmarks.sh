#!/bin/bash

# Script to run Dilithium benchmarks across all parameter sets
# This script saves all output to benchmark_results.log in the dilithium root.

# Resolve repository root (directory containing this script)
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REF_DIR="$REPO_ROOT/ref"
LOG="$REPO_ROOT/benchmark_results.log"

echo "Building and running Dilithium benchmarks..."

# Start/overwrite the log
echo "=== Dilithium benchmark run ===" > "$LOG"
echo "Started: $(date -u)" >> "$LOG"
echo "" >> "$LOG"

# CD to the reference implementation directory
cd "$REF_DIR" || exit 1

# Build all parameter sets and log output
echo "Building benchmarks..." | tee -a "$LOG"
echo "--- make build output ---" >> "$LOG"
make test/test_time2 test/test_time3 test/test_time5 >> "$LOG" 2>&1
echo "--- end build output ---" >> "$LOG"

# Run benchmarks and append output to the log
echo >> "$LOG"
echo "=== Dilithium Mode 2 ===" >> "$LOG"
echo "Running: ./test/test_time2" >> "$LOG"
./test/test_time2 >> "$LOG" 2>&1

echo >> "$LOG"
echo "=== Dilithium Mode 3 ===" >> "$LOG"
echo "Running: ./test/test_time3" >> "$LOG"
./test/test_time3 >> "$LOG" 2>&1

echo >> "$LOG"
echo "=== Dilithium Mode 5 ===" >> "$LOG"
echo "Running: ./test/test_time5" >> "$LOG"
./test/test_time5 >> "$LOG" 2>&1

echo "" >> "$LOG"
echo "Finished: $(date -u)" >> "$LOG"

echo "All done. Results saved to $LOG"