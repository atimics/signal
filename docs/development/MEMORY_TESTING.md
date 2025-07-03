# Memory Testing Configuration

## Setup Instructions

1. Install Valgrind (on Linux/Mac via homebrew):
   ```bash
   # macOS
   brew install valgrind
   
   # Linux
   sudo apt-get install valgrind
   ```

2. Add memory test targets to Makefile

## Memory Test Execution

### Memory Leak Detection
```bash
make memory-test
```

### Memory Error Detection  
```bash
make memory-check
```

### Address Sanitizer (Fast)
```bash
make asan-test
```

## Expected Results

- **Zero memory leaks** in all test suites
- **Zero memory errors** (double free, use after free, etc.)
- **Clean exit** for all test executables

## Common Issues

1. **Component Pool Cleanup**: Ensure world_destroy properly frees component pools
2. **Asset Cleanup**: Graphics resources must be released
3. **System Shutdown**: Input/control systems need proper shutdown

## CI Integration

Memory tests should be part of the CI pipeline to prevent regression.
