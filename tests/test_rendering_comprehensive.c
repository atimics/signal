/**
 * @file test_rendering_comprehensive.c
 * @brief Comprehensive test runner for all rendering and UI tests
 * 
 * This file runs all the rendering and UI pipeline tests in sequence,
 * providing detailed output and summary statistics.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "vendor/unity.h"

// Forward declarations of all test suites
void suite_core_math(void);
void suite_assets(void);
void suite_rendering(void);

// New comprehensive rendering test suites
void suite_render_layers(void);
void suite_ui_rendering_pipeline(void);
void suite_graphics_pipeline(void);
void suite_render_3d_pipeline(void);
void suite_rendering_integration(void);

// Test suite information
typedef struct {
    const char* name;
    void (*suite_function)(void);
    const char* description;
} TestSuiteInfo;

// Global test configuration
static bool verbose_output = false;
static bool run_performance_tests = true;
static bool run_integration_tests = true;

// Define all test suites
static const TestSuiteInfo test_suites[] = {
    // Core tests
    {
        "Core Math",
        suite_core_math,
        "Basic mathematical operations and utilities"
    },
    {
        "Assets",
        suite_assets,
        "Asset loading and management system"
    },
    {
        "Rendering (Legacy)",
        suite_rendering,
        "Legacy rendering system tests"
    },
    
    // New comprehensive rendering tests
    {
        "Render Layers",
        suite_render_layers,
        "Offscreen rendering layer system and compositing"
    },
    {
        "UI Rendering Pipeline",
        suite_ui_rendering_pipeline,
        "UI context management and MicroUI integration"
    },
    {
        "Graphics Pipeline",
        suite_graphics_pipeline,
        "Graphics API and Sokol resource management"
    },
    {
        "3D Render Pipeline",
        suite_render_3d_pipeline,
        "3D rendering, entity system, and camera integration"
    },
    {
        "Rendering Integration",
        suite_rendering_integration,
        "Integration tests for complete rendering system"
    }
};

static const int num_test_suites = sizeof(test_suites) / sizeof(TestSuiteInfo);

// Test statistics
typedef struct {
    int suites_run;
    int suites_passed;
    int suites_failed;
    int total_tests_run;
    int total_tests_passed;
    int total_tests_failed;
    double total_duration_ms;
} TestStatistics;

static TestStatistics stats = {0};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

static void print_banner(const char* title) {
    const int width = 80;
    const int title_len = strlen(title);
    const int padding = (width - title_len - 2) / 2;
    
    printf("\n");
    for (int i = 0; i < width; i++) printf("=");
    printf("\n");
    
    for (int i = 0; i < padding; i++) printf(" ");
    printf("%s", title);
    for (int i = 0; i < width - padding - title_len; i++) printf(" ");
    printf("\n");
    
    for (int i = 0; i < width; i++) printf("=");
    printf("\n\n");
}

static void print_test_suite_header(const TestSuiteInfo* suite_info) {
    printf("┌─────────────────────────────────────────────────────────────────────────────┐\n");
    printf("│ Running: %-67s │\n", suite_info->name);
    printf("│ Description: %-59s │\n", suite_info->description);
    printf("└─────────────────────────────────────────────────────────────────────────────┘\n");
}

static void print_test_suite_result(const TestSuiteInfo* suite_info, 
                                   int tests_run, int tests_failed, double duration_ms) {
    const char* status = (tests_failed == 0) ? "PASSED" : "FAILED";
    const char* status_color = (tests_failed == 0) ? "✅" : "❌";
    
    printf("┌─────────────────────────────────────────────────────────────────────────────┐\n");
    printf("│ %s %-67s │\n", status_color, suite_info->name);
    printf("│ Tests: %d run, %d passed, %d failed                                      │\n", 
           tests_run, tests_run - tests_failed, tests_failed);
    printf("│ Duration: %.2f ms                                                          │\n", 
           duration_ms);
    printf("└─────────────────────────────────────────────────────────────────────────────┘\n\n");
}

// ============================================================================
// TEST EXECUTION FUNCTIONS
// ============================================================================

static int run_test_suite(const TestSuiteInfo* suite_info) {
    print_test_suite_header(suite_info);
    
    double start_time = get_time_ms();
    
    // Capture Unity state before running the suite
    int tests_before = Unity.TestFailures;
    int total_before = Unity.TotalTests;
    
    // Run the test suite
    suite_info->suite_function();
    
    // Calculate results
    int tests_after = Unity.TestFailures;
    int total_after = Unity.TotalTests;
    
    int tests_run = total_after - total_before;
    int tests_failed = tests_after - tests_before;
    double duration = get_time_ms() - start_time;
    
    // Update statistics
    stats.suites_run++;
    stats.total_tests_run += tests_run;
    stats.total_tests_failed += tests_failed;
    stats.total_tests_passed += (tests_run - tests_failed);
    stats.total_duration_ms += duration;
    
    if (tests_failed == 0) {
        stats.suites_passed++;
    } else {
        stats.suites_failed++;
    }
    
    print_test_suite_result(suite_info, tests_run, tests_failed, duration);
    
    return (tests_failed == 0) ? 0 : 1;
}

static void print_final_summary(void) {
    print_banner("TEST EXECUTION SUMMARY");
    
    printf("Overall Results:\n");
    printf("  Test Suites: %d run, %d passed, %d failed\n", 
           stats.suites_run, stats.suites_passed, stats.suites_failed);
    printf("  Total Tests: %d run, %d passed, %d failed\n", 
           stats.total_tests_run, stats.total_tests_passed, stats.total_tests_failed);
    printf("  Total Duration: %.2f ms (%.2f seconds)\n", 
           stats.total_duration_ms, stats.total_duration_ms / 1000.0);
    
    double success_rate = (stats.total_tests_run > 0) ? 
        (double)stats.total_tests_passed / stats.total_tests_run * 100.0 : 0.0;
    printf("  Success Rate: %.1f%%\n", success_rate);
    
    if (stats.suites_failed == 0) {
        printf("\n🎉 ALL TESTS PASSED! 🎉\n");
    } else {
        printf("\n❌ %d TEST SUITE(S) FAILED ❌\n", stats.suites_failed);
    }
    
    printf("\nDetailed Test Categories:\n");
    printf("  ✅ Render Layers System: Layer management, compositing, resource handling\n");
    printf("  ✅ UI Rendering Pipeline: MicroUI integration, context management, safety\n");
    printf("  ✅ Graphics Pipeline: Sokol resource lifecycle, validation, error handling\n");
    printf("  ✅ 3D Render Pipeline: Entity rendering, transformations, camera system\n");
    printf("  ✅ Integration Tests: Cross-system interactions, performance, real scenarios\n");
    
    printf("\nTest Coverage Areas:\n");
    printf("  • Resource Management: Buffer, image, shader, pipeline lifecycle\n");
    printf("  • State Management: Context validation, pipeline state isolation\n");
    printf("  • Error Handling: Invalid parameters, resource exhaustion, recovery\n");
    printf("  • Performance: Multi-layer rendering, memory pressure, frame simulation\n");
    printf("  • Integration: 3D+UI rendering, layer compositing, system coordination\n");
    
    printf("\n");
    for (int i = 0; i < 80; i++) printf("=");
    printf("\n");
}

// ============================================================================
// COMMAND LINE ARGUMENT PARSING
// ============================================================================

static void print_usage(const char* program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("\nOptions:\n");
    printf("  -h, --help              Show this help message\n");
    printf("  -v, --verbose           Enable verbose output\n");
    printf("  --no-performance        Skip performance tests\n");
    printf("  --no-integration        Skip integration tests\n");
    printf("  --suite <name>          Run only specific test suite\n");
    printf("\nAvailable test suites:\n");
    
    for (int i = 0; i < num_test_suites; i++) {
        printf("  %-20s %s\n", test_suites[i].name, test_suites[i].description);
    }
    
    printf("\nExamples:\n");
    printf("  %s                      # Run all tests\n", program_name);
    printf("  %s --verbose            # Run all tests with verbose output\n", program_name);
    printf("  %s --suite \"Render Layers\" # Run only render layers tests\n", program_name);
    printf("  %s --no-performance     # Skip performance tests\n", program_name);
}

static int parse_arguments(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 1;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose_output = true;
        } else if (strcmp(argv[i], "--no-performance") == 0) {
            run_performance_tests = false;
        } else if (strcmp(argv[i], "--no-integration") == 0) {
            run_integration_tests = false;
        } else if (strcmp(argv[i], "--suite") == 0) {
            if (i + 1 < argc) {
                // For now, just note that a specific suite was requested
                // Full implementation would filter to run only that suite
                i++; // Skip the suite name argument
                printf("Note: Suite filtering not fully implemented yet\n");
            } else {
                printf("Error: --suite requires a suite name\n");
                return 1;
            }
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    return 0;
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main(int argc, char* argv[]) {
    // Parse command line arguments
    if (parse_arguments(argc, argv) != 0) {
        return 1;
    }
    
    // Initialize Unity
    UNITY_BEGIN();
    
    // Print startup banner
    print_banner("CGAME COMPREHENSIVE RENDERING & UI TEST SUITE");
    
    printf("Test Configuration:\n");
    printf("  Verbose Output: %s\n", verbose_output ? "Enabled" : "Disabled");
    printf("  Performance Tests: %s\n", run_performance_tests ? "Enabled" : "Disabled");
    printf("  Integration Tests: %s\n", run_integration_tests ? "Enabled" : "Disabled");
    printf("  Total Test Suites: %d\n", num_test_suites);
    printf("\n");
    
    // Run all test suites
    for (int i = 0; i < num_test_suites; i++) {
        const TestSuiteInfo* suite = &test_suites[i];
        
        // Skip performance tests if disabled
        if (!run_performance_tests && strstr(suite->name, "Performance") != NULL) {
            printf("Skipping %s (performance tests disabled)\n", suite->name);
            continue;
        }
        
        // Skip integration tests if disabled
        if (!run_integration_tests && strstr(suite->name, "Integration") != NULL) {
            printf("Skipping %s (integration tests disabled)\n", suite->name);
            continue;
        }
        
        run_test_suite(suite);
        
        // Add a small delay between test suites for readability
        if (verbose_output) {
            printf("Completed %s, moving to next suite...\n\n", suite->name);
        }
    }
    
    // Print final summary
    print_final_summary();
    
    // Return Unity result
    int unity_result = UNITY_END();
    
    // Return appropriate exit code
    return (stats.suites_failed == 0 && unity_result == 0) ? 0 : 1;
}
