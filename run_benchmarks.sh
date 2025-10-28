#!/bin/bash

# Script to run Dilithium benchmarks across all parameter sets
echo "Building and running Dilithium benchmarks..."

# CD to the reference implementation directory
cd "$(dirname "$0")/ref" || exit 1

# Build all parameter sets
echo "Building benchmarks..."
make test/test_time2 test/test_time3 test/test_time5

# Run benchmarks
echo -e "\n=== Dilithium Mode 2 ==="
./test/test_time2

echo -e "\n=== Dilithium Mode 3 ==="
./test/test_time3

echo -e "\n=== Dilithium Mode 5 ==="
./test/test_time5