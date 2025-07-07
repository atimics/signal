/**
 * @file test_discovery.c
 * @brief Implementation of automated test discovery system
 * 
 * This is a simplified implementation demonstrating the core concepts
 * of the test discovery system outlined in our QA organization.
 */

#include "test_discovery.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// ============================================================================
// GLOBAL STATE
// ============================================================================

static TestMetadata* g_registered_tests = NULL;
static uint32_t g_test_count = 0;
static uint32_t g_test_capacity = 0;

static TestSuiteMetadata* g_registered_suites = NULL;
static uint32_t g_suite_count = 0;
static uint32_t g_suite_capacity = 0;

static bool g_discovery_initialized = false;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static bool ensure_test_capacity(uint32_t needed_capacity) {
    if (needed_capacity <= g_test_capacity) {
        return true;
    }
    
    uint32_t new_capacity = g_test_capacity == 0 ? 64 : g_test_capacity * 2;
    while (new_capacity < needed_capacity) {
        new_capacity *= 2;
    }
    
    TestMetadata* new_tests = realloc(g_registered_tests, 
                                     new_capacity * sizeof(TestMetadata));
    if (!new_tests) {
        return false;
    }
    
    g_registered_tests = new_tests;
    g_test_capacity = new_capacity;
    return true;
}

static bool ensure_suite_capacity(uint32_t needed_capacity) {
    if (needed_capacity <= g_suite_capacity) {
        return true;
    }
    
    uint32_t new_capacity = g_suite_capacity == 0 ? 32 : g_suite_capacity * 2;
    while (new_capacity < needed_capacity) {
        new_capacity *= 2;
    }
    
    TestSuiteMetadata* new_suites = realloc(g_registered_suites,
                                          new_capacity * sizeof(TestSuiteMetadata));
    if (!new_suites) {
        return false;
    }
    
    g_registered_suites = new_suites;
    g_suite_capacity = new_capacity;
    return true;
}

// ============================================================================
// PUBLIC API IMPLEMENTATION
// ============================================================================

bool test_discovery_init(void) {
    if (g_discovery_initialized) {
        return true;
    }
    
    // Initialize arrays
    if (!ensure_test_capacity(64) || !ensure_suite_capacity(32)) {
        return false;
    }
    
    g_test_count = 0;
    g_suite_count = 0;
    g_discovery_initialized = true;
    
    printf("🔍 Test Discovery System Initialized\n");
    printf("   Initial capacity: %u tests, %u suites\n", 
           g_test_capacity, g_suite_capacity);
    
    return true;
}

void test_discovery_shutdown(void) {
    if (!g_discovery_initialized) {
        return;
    }
    
    free(g_registered_tests);
    free(g_registered_suites);
    
    g_registered_tests = NULL;
    g_registered_suites = NULL;
    g_test_count = 0;
    g_suite_count = 0;
    g_test_capacity = 0;
    g_suite_capacity = 0;
    g_discovery_initialized = false;
    
    printf("🔍 Test Discovery System Shutdown\n");
}

bool test_discovery_register_test(const TestMetadata* metadata) {
    if (!g_discovery_initialized) {
        fprintf(stderr, "❌ Test discovery not initialized\n");
        return false;
    }
    
    if (!metadata || !metadata->name || !metadata->function) {
        fprintf(stderr, "❌ Invalid test metadata\n");
        return false;
    }
    
    // Check for duplicate registration
    for (uint32_t i = 0; i < g_test_count; i++) {
        if (strcmp(g_registered_tests[i].name, metadata->name) == 0) {
            fprintf(stderr, "⚠️  Test '%s' already registered\n", metadata->name);
            return true; // Not an error, just a duplicate
        }
    }
    
    if (!ensure_test_capacity(g_test_count + 1)) {
        fprintf(stderr, "❌ Failed to allocate memory for test registration\n");
        return false;
    }
    
    // Copy metadata
    g_registered_tests[g_test_count] = *metadata;
    
    // Duplicate string fields to ensure they persist
    g_registered_tests[g_test_count].name = strdup(metadata->name);
    g_registered_tests[g_test_count].description = metadata->description ? 
        strdup(metadata->description) : strdup("No description");
    g_registered_tests[g_test_count].file = metadata->file ? 
        strdup(metadata->file) : strdup("Unknown file");
    g_registered_tests[g_test_count].tags = metadata->tags ? 
        strdup(metadata->tags) : strdup("");
    
    g_test_count++;
    
    printf("✅ Registered test: %s [%s] (%s)\n", 
           metadata->name, 
           test_category_to_string(metadata->category),
           metadata->description ? metadata->description : "No description");
    
    return true;
}

bool test_discovery_register_suite(const TestSuiteMetadata* suite_metadata) {
    if (!g_discovery_initialized) {
        fprintf(stderr, "❌ Test discovery not initialized\n");
        return false;
    }
    
    if (!suite_metadata || !suite_metadata->name || !suite_metadata->function) {
        fprintf(stderr, "❌ Invalid suite metadata\n");
        return false;
    }
    
    // Check for duplicate registration
    for (uint32_t i = 0; i < g_suite_count; i++) {
        if (strcmp(g_registered_suites[i].name, suite_metadata->name) == 0) {
            fprintf(stderr, "⚠️  Suite '%s' already registered\n", suite_metadata->name);
            return true; // Not an error, just a duplicate
        }
    }
    
    if (!ensure_suite_capacity(g_suite_count + 1)) {
        fprintf(stderr, "❌ Failed to allocate memory for suite registration\n");
        return false;
    }
    
    // Copy metadata
    g_registered_suites[g_suite_count] = *suite_metadata;
    
    // Duplicate string fields
    g_registered_suites[g_suite_count].name = strdup(suite_metadata->name);
    g_registered_suites[g_suite_count].description = suite_metadata->description ?
        strdup(suite_metadata->description) : strdup("No description");
    g_registered_suites[g_suite_count].module = suite_metadata->module ?
        strdup(suite_metadata->module) : strdup("Unknown module");
    
    g_suite_count++;
    
    printf("📋 Registered suite: %s [%s] (%s)\n",
           suite_metadata->name,
           suite_metadata->module ? suite_metadata->module : "Unknown module",
           suite_metadata->description ? suite_metadata->description : "No description");
    
    return true;
}

bool test_discovery_get_tests(TestMetadata** out_tests, uint32_t* out_count) {
    if (!g_discovery_initialized) {
        return false;
    }
    
    if (!out_tests || !out_count) {
        return false;
    }
    
    *out_tests = g_registered_tests;
    *out_count = g_test_count;
    return true;
}

bool test_discovery_get_suites(TestSuiteMetadata** out_suites, uint32_t* out_count) {
    if (!g_discovery_initialized) {
        return false;
    }
    
    if (!out_suites || !out_count) {
        return false;
    }
    
    *out_suites = g_registered_suites;
    *out_count = g_suite_count;
    return true;
}

bool test_discovery_get_stats(TestDiscoveryStats* out_stats) {
    if (!g_discovery_initialized || !out_stats) {
        return false;
    }
    
    memset(out_stats, 0, sizeof(TestDiscoveryStats));
    
    out_stats->total_tests = g_test_count;
    out_stats->total_suites = g_suite_count;
    
    // Count by category and priority
    for (uint32_t i = 0; i < g_test_count; i++) {
        TestMetadata* test = &g_registered_tests[i];
        
        if (test->category < TEST_CATEGORY_COUNT) {
            out_stats->tests_by_category[test->category]++;
        }
        
        if (test->priority < TEST_PRIORITY_COUNT) {
            out_stats->tests_by_priority[test->priority]++;
        }
        
        if (test->enabled) {
            out_stats->enabled_tests++;
        } else {
            out_stats->disabled_tests++;
        }
    }
    
    return true;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

const char* test_category_to_string(TestCategory category) {
    switch (category) {
        case TEST_CATEGORY_UNIT: return "UNIT";
        case TEST_CATEGORY_INTEGRATION: return "INTEGRATION";
        case TEST_CATEGORY_PERFORMANCE: return "PERFORMANCE";
        case TEST_CATEGORY_REGRESSION: return "REGRESSION";
        case TEST_CATEGORY_SMOKE: return "SMOKE";
        case TEST_CATEGORY_ACCEPTANCE: return "ACCEPTANCE";
        case TEST_CATEGORY_EXPERIMENTAL: return "EXPERIMENTAL";
        default: return "UNKNOWN";
    }
}

const char* test_priority_to_string(TestPriority priority) {
    switch (priority) {
        case TEST_PRIORITY_CRITICAL: return "CRITICAL";
        case TEST_PRIORITY_HIGH: return "HIGH";
        case TEST_PRIORITY_MEDIUM: return "MEDIUM";
        case TEST_PRIORITY_LOW: return "LOW";
        default: return "UNKNOWN";
    }
}

TestCategory test_category_from_string(const char* category_str) {
    if (!category_str) return TEST_CATEGORY_UNIT;
    
    if (strcmp(category_str, "UNIT") == 0) return TEST_CATEGORY_UNIT;
    if (strcmp(category_str, "INTEGRATION") == 0) return TEST_CATEGORY_INTEGRATION;
    if (strcmp(category_str, "PERFORMANCE") == 0) return TEST_CATEGORY_PERFORMANCE;
    if (strcmp(category_str, "REGRESSION") == 0) return TEST_CATEGORY_REGRESSION;
    if (strcmp(category_str, "SMOKE") == 0) return TEST_CATEGORY_SMOKE;
    if (strcmp(category_str, "ACCEPTANCE") == 0) return TEST_CATEGORY_ACCEPTANCE;
    if (strcmp(category_str, "EXPERIMENTAL") == 0) return TEST_CATEGORY_EXPERIMENTAL;
    
    return TEST_CATEGORY_UNIT; // Default
}

TestPriority test_priority_from_string(const char* priority_str) {
    if (!priority_str) return TEST_PRIORITY_MEDIUM;
    
    if (strcmp(priority_str, "CRITICAL") == 0) return TEST_PRIORITY_CRITICAL;
    if (strcmp(priority_str, "HIGH") == 0) return TEST_PRIORITY_HIGH;
    if (strcmp(priority_str, "MEDIUM") == 0) return TEST_PRIORITY_MEDIUM;
    if (strcmp(priority_str, "LOW") == 0) return TEST_PRIORITY_LOW;
    
    return TEST_PRIORITY_MEDIUM; // Default
}

bool test_has_tag(const TestMetadata* test, const char* tag) {
    if (!test || !test->tags || !tag) {
        return false;
    }
    
    return strstr(test->tags, tag) != NULL;
}

void test_discovery_print_stats(const TestDiscoveryStats* stats) {
    if (!stats) {
        return;
    }
    
    printf("\n📊 Test Discovery Statistics\n");
    printf("============================\n");
    printf("Total Tests: %u\n", stats->total_tests);
    printf("Total Suites: %u\n", stats->total_suites);
    printf("Enabled Tests: %u\n", stats->enabled_tests);
    printf("Disabled Tests: %u\n", stats->disabled_tests);
    
    printf("\nBy Category:\n");
    for (int i = 0; i < TEST_CATEGORY_COUNT; i++) {
        if (stats->tests_by_category[i] > 0) {
            printf("  %s: %u\n", 
                   test_category_to_string((TestCategory)i), 
                   stats->tests_by_category[i]);
        }
    }
    
    printf("\nBy Priority:\n");
    for (int i = 0; i < TEST_PRIORITY_COUNT; i++) {
        if (stats->tests_by_priority[i] > 0) {
            printf("  %s: %u\n", 
                   test_priority_to_string((TestPriority)i), 
                   stats->tests_by_priority[i]);
        }
    }
    printf("\n");
}

bool test_discovery_generate_report(const char* output_file) {
    if (!g_discovery_initialized) {
        return false;
    }
    
    FILE* f = output_file ? fopen(output_file, "w") : stdout;
    if (!f) {
        return false;
    }
    
    fprintf(f, "# Test Discovery Report\n\n");
    fprintf(f, "Generated: %s\n", __DATE__ " " __TIME__);
    fprintf(f, "Total Tests: %u\n", g_test_count);
    fprintf(f, "Total Suites: %u\n\n", g_suite_count);
    
    // List all tests
    fprintf(f, "## Registered Tests\n\n");
    for (uint32_t i = 0; i < g_test_count; i++) {
        TestMetadata* test = &g_registered_tests[i];
        fprintf(f, "- **%s** [%s/%s] %s\n",
                test->name,
                test_category_to_string(test->category),
                test_priority_to_string(test->priority),
                test->enabled ? "✅" : "❌");
        if (test->description && strlen(test->description) > 0) {
            fprintf(f, "  %s\n", test->description);
        }
        if (test->tags && strlen(test->tags) > 0) {
            fprintf(f, "  Tags: %s\n", test->tags);
        }
        fprintf(f, "\n");
    }
    
    // List all suites
    fprintf(f, "## Registered Suites\n\n");
    for (uint32_t i = 0; i < g_suite_count; i++) {
        TestSuiteMetadata* suite = &g_registered_suites[i];
        fprintf(f, "- **%s** [%s] %s\n",
                suite->name,
                suite->module ? suite->module : "Unknown",
                suite->enabled ? "✅" : "❌");
        if (suite->description && strlen(suite->description) > 0) {
            fprintf(f, "  %s\n", suite->description);
        }
        fprintf(f, "\n");
    }
    
    if (output_file) {
        fclose(f);
    }
    
    return true;
}

// ============================================================================
// STUB IMPLEMENTATIONS (to be completed in full implementation)
// ============================================================================

bool test_discovery_scan_directory(const char* test_directory) {
    // This would be implemented to scan source files for test functions
    printf("🔍 Would scan directory: %s\n", test_directory);
    return true;
}

bool test_discovery_get_tests_by_category(TestCategory category, 
                                         TestMetadata** out_tests, 
                                         uint32_t* out_count) {
    // Filter tests by category - implementation would create filtered array
    printf("🔍 Would filter %u tests by category %s\n", 
           g_test_count, test_category_to_string(category));
    return false; // Stub
}

bool test_discovery_get_tests_by_priority(TestPriority priority,
                                         TestMetadata** out_tests,
                                         uint32_t* out_count) {
    // Filter tests by priority - implementation would create filtered array
    printf("🔍 Would filter %u tests by priority %s\n", 
           g_test_count, test_priority_to_string(priority));
    return false; // Stub
}

bool test_discovery_get_tests_by_tag(const char* tag,
                                    TestMetadata** out_tests,
                                    uint32_t* out_count) {
    // Filter tests by tag - implementation would create filtered array
    printf("🔍 Would filter %u tests by tag '%s'\n", g_test_count, tag);
    return false; // Stub
}

bool test_discovery_set_test_enabled(const char* test_name, bool enabled) {
    for (uint32_t i = 0; i < g_test_count; i++) {
        if (strcmp(g_registered_tests[i].name, test_name) == 0) {
            g_registered_tests[i].enabled = enabled;
            printf("🔧 Test '%s' %s\n", test_name, enabled ? "enabled" : "disabled");
            return true;
        }
    }
    return false;
}

bool test_discovery_set_suite_enabled(const char* suite_name, bool enabled) {
    for (uint32_t i = 0; i < g_suite_count; i++) {
        if (strcmp(g_registered_suites[i].name, suite_name) == 0) {
            g_registered_suites[i].enabled = enabled;
            printf("🔧 Suite '%s' %s\n", suite_name, enabled ? "enabled" : "disabled");
            return true;
        }
    }
    return false;
}
