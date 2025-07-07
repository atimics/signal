// tests/stubs/memory_test_stubs.c
// Minimal stubs for isolated memory performance testing
// Provides missing symbols without full graphics/asset system dependencies

#include <stdio.h>
#include <stddef.h>

// Stub asset registry - just a simple structure
typedef struct {
    int dummy;
} AssetRegistry;

static AssetRegistry stub_registry = {0};
AssetRegistry* g_asset_registry = &stub_registry;

// Stub performance tracking
void performance_record_memory_usage(size_t bytes_allocated, size_t bytes_freed) {
    // Simple stub - just ignore the call
    (void)bytes_allocated;
    (void)bytes_freed;
}
