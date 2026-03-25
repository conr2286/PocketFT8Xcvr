#!/bin/bash
# Sequential hardware test execution with proper delays
# Usage: ./run_hw_tests.sh

echo "Running hardware tests sequentially..."

# Array of test directories in order
tests=("hw/test_01" "hw/test_02" "hw/test_03" "hw/test_04" "hw/test_05")

for test in "${tests[@]}"; do
    echo "=========================================="
    echo "Running $test"
    echo "=========================================="
    
    # Run the test
    pio test -v -f "$test"
    
    # Check if test failed
    if [ $? -ne 0 ]; then
        echo "ERROR: Test $test failed!"
        exit 1
    fi
    
    echo "Test $test completed successfully"
    
    # Wait for USB enumeration between tests
    echo "Waiting for USB port to stabilize..."
    sleep 3
done

echo "All hardware tests completed successfully!"