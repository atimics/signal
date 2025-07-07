/**
 * @file test_runner.c
 * @brief Comprehensive test aggregation and coverage reporting system
 * 
 * Runs all test suites, aggregates results, generates reports in multiple formats,
 * and provides code coverage analysis using gcov/lcov integration.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>

// Test result tracking
typedef struct {
    char name[256];
    int passed;
    int failed;
    int total;
    double duration_ms;
    char error_details[1024];
} TestSuiteResult;

// Global test configuration
typedef struct {
    bool verbose;
    bool html_report;
    bool junit_xml;
    bool coverage;
    bool filter_enabled;
    char filter_pattern[256];
    char output_dir[512];
} TestConfig;

// Test suite definitions
static const char* test_suites[] = {
    "./build/test_ui_system",
    "./build/test_ui_visibility", 
    "./build/test_bitmap_font",
    "./build/test_ui_events",
    "./build/test_ui_performance",
    "./build/test_microui_diagnostics",
    "./build/test_microui_pipeline",
    "./build/test_microui_minimal",
    "./build/test_thrusters",
    "./build/test_physics",
    "./build/test_transform",
    "./build/test_entity_manager",
    "./build/test_component_registry",
    "./build/test_input_calibration",
    "./build/test_input_filtering",
    "./build/test_scene_management",
    "./build/test_flight_controls",
    "./build/test_performance"
};

static const int num_test_suites = sizeof(test_suites) / sizeof(test_suites[0]);
static TestSuiteResult results[32];  // Max 32 test suites
static TestConfig config = {0};

// Utility functions
static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

static bool file_exists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0;
}

static void create_directory(const char* path) {
    mkdir(path, 0755);
}

static bool should_run_suite(const char* suite_name) {
    if (!config.filter_enabled) return true;
    return strstr(suite_name, config.filter_pattern) != NULL;
}

// Test execution
static int run_test_suite(const char* executable, TestSuiteResult* result) {
    if (!file_exists(executable)) {
        printf("⚠️  Test suite not found: %s\n", executable);
        strcpy(result->name, executable);
        result->passed = 0;
        result->failed = 1;
        result->total = 1;
        result->duration_ms = 0.0;
        strcpy(result->error_details, "Executable not found");
        return -1;
    }

    printf("🧪 Running %s...\n", executable);
    
    double start_time = get_time_ms();
    
    // Create pipe to capture output
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        // Child process - run test
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        execl(executable, executable, (char*)NULL);
        perror("execl");
        exit(1);
    } else {
        // Parent process - capture output
        close(pipefd[1]);
        
        char buffer[4096];
        char full_output[8192] = {0};
        ssize_t bytes_read;
        
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            if (strlen(full_output) + bytes_read < sizeof(full_output) - 1) {
                strcat(full_output, buffer);
            }
            if (config.verbose) {
                printf("%s", buffer);
            }
        }
        close(pipefd[0]);
        
        int status;
        waitpid(pid, &status, 0);
        
        double end_time = get_time_ms();
        result->duration_ms = end_time - start_time;
        
        // Parse Unity test output
        strcpy(result->name, executable);
        result->passed = 0;
        result->failed = 0;
        result->total = 0;
        strcpy(result->error_details, "");
        
        // Parse test results from output
        char* line = strtok(full_output, "\n");
        while (line != NULL) {
            // Unity format: "X Tests Y Failures Z Ignored"
            if (strstr(line, "Tests") && strstr(line, "Failures")) {
                sscanf(line, "%d Tests %d Failures", &result->total, &result->failed);
                result->passed = result->total - result->failed;
            }
            
            // Capture error details
            if (strstr(line, "FAIL:") || strstr(line, "ERROR:") || strstr(line, "Assertion failed")) {
                if (strlen(result->error_details) < sizeof(result->error_details) - 200) {
                    strcat(result->error_details, line);
                    strcat(result->error_details, "\n");
                }
            }
            
            line = strtok(NULL, "\n");
        }
        
        // If no Unity output found, assume binary test result
        if (result->total == 0) {
            result->total = 1;
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                result->passed = 1;
                result->failed = 0;
            } else {
                result->passed = 0;
                result->failed = 1;
                snprintf(result->error_details, sizeof(result->error_details), 
                        "Test exited with code %d", WEXITSTATUS(status));
            }
        }
        
        return WEXITSTATUS(status);
    }
}

// Report generation
static void generate_console_report(TestSuiteResult* results, int count) {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("                        TEST RESULTS SUMMARY                   \n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    int total_passed = 0, total_failed = 0, total_tests = 0;
    double total_time = 0.0;
    
    for (int i = 0; i < count; i++) {
        total_passed += results[i].passed;
        total_failed += results[i].failed;
        total_tests += results[i].total;
        total_time += results[i].duration_ms;
        
        const char* status = results[i].failed == 0 ? "✅ PASS" : "❌ FAIL";
        const char* suite_name = strrchr(results[i].name, '/');
        suite_name = suite_name ? suite_name + 1 : results[i].name;
        
        printf("%-30s %s  %3d/%3d  %6.1fms\n", 
               suite_name, status, results[i].passed, results[i].total, results[i].duration_ms);
        
        if (results[i].failed > 0 && strlen(results[i].error_details) > 0) {
            printf("    Error: %s\n", results[i].error_details);
        }
    }
    
    printf("───────────────────────────────────────────────────────────────\n");
    printf("TOTAL:                         %3d/%3d  %6.1fms\n", 
           total_passed, total_tests, total_time);
    
    double success_rate = total_tests > 0 ? (double)total_passed / total_tests * 100.0 : 0.0;
    printf("SUCCESS RATE: %.1f%%\n", success_rate);
    
    if (total_failed > 0) {
        printf("❌ %d test(s) failed\n", total_failed);
    } else {
        printf("✅ All tests passed!\n");
    }
    printf("═══════════════════════════════════════════════════════════════\n");
}

static void generate_html_report(TestSuiteResult* results, int count) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/test_report.html", config.output_dir);
    
    FILE* f = fopen(filepath, "w");
    if (!f) {
        printf("⚠️  Could not create HTML report: %s\n", filepath);
        return;
    }
    
    fprintf(f, "<!DOCTYPE html>\n<html>\n<head>\n");
    fprintf(f, "<title>SIGNAL Test Results</title>\n");
    fprintf(f, "<style>\n");
    fprintf(f, "body { font-family: Arial, sans-serif; margin: 20px; }\n");
    fprintf(f, ".header { background: #f0f0f0; padding: 20px; border-radius: 5px; }\n");
    fprintf(f, ".summary { margin: 20px 0; }\n");
    fprintf(f, ".test-suite { margin: 10px 0; padding: 10px; border: 1px solid #ddd; }\n");
    fprintf(f, ".pass { background-color: #d4edda; }\n");
    fprintf(f, ".fail { background-color: #f8d7da; }\n");
    fprintf(f, ".error-details { margin-top: 10px; font-family: monospace; background: #f8f9fa; padding: 5px; }\n");
    fprintf(f, "table { border-collapse: collapse; width: 100%%; }\n");
    fprintf(f, "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n");
    fprintf(f, "th { background-color: #f2f2f2; }\n");
    fprintf(f, "</style>\n</head>\n<body>\n");
    
    // Header
    time_t now = time(NULL);
    fprintf(f, "<div class='header'>\n");
    fprintf(f, "<h1>SIGNAL Test Results</h1>\n");
    fprintf(f, "<p>Generated: %s</p>\n", ctime(&now));
    fprintf(f, "</div>\n");
    
    // Summary
    int total_passed = 0, total_failed = 0, total_tests = 0;
    double total_time = 0.0;
    
    for (int i = 0; i < count; i++) {
        total_passed += results[i].passed;
        total_failed += results[i].failed;
        total_tests += results[i].total;
        total_time += results[i].duration_ms;
    }
    
    double success_rate = total_tests > 0 ? (double)total_passed / total_tests * 100.0 : 0.0;
    
    fprintf(f, "<div class='summary'>\n");
    fprintf(f, "<h2>Summary</h2>\n");
    fprintf(f, "<table>\n");
    fprintf(f, "<tr><th>Metric</th><th>Value</th></tr>\n");
    fprintf(f, "<tr><td>Total Tests</td><td>%d</td></tr>\n", total_tests);
    fprintf(f, "<tr><td>Passed</td><td>%d</td></tr>\n", total_passed);
    fprintf(f, "<tr><td>Failed</td><td>%d</td></tr>\n", total_failed);
    fprintf(f, "<tr><td>Success Rate</td><td>%.1f%%</td></tr>\n", success_rate);
    fprintf(f, "<tr><td>Total Time</td><td>%.1f ms</td></tr>\n", total_time);
    fprintf(f, "</table>\n");
    fprintf(f, "</div>\n");
    
    // Detailed results
    fprintf(f, "<h2>Detailed Results</h2>\n");
    fprintf(f, "<table>\n");
    fprintf(f, "<tr><th>Test Suite</th><th>Status</th><th>Passed</th><th>Failed</th><th>Duration (ms)</th></tr>\n");
    
    for (int i = 0; i < count; i++) {
        const char* suite_name = strrchr(results[i].name, '/');
        suite_name = suite_name ? suite_name + 1 : results[i].name;
        const char* status = results[i].failed == 0 ? "PASS" : "FAIL";
        const char* row_class = results[i].failed == 0 ? "pass" : "fail";
        
        fprintf(f, "<tr class='%s'>\n", row_class);
        fprintf(f, "<td>%s</td>\n", suite_name);
        fprintf(f, "<td>%s</td>\n", status);
        fprintf(f, "<td>%d</td>\n", results[i].passed);
        fprintf(f, "<td>%d</td>\n", results[i].failed);
        fprintf(f, "<td>%.1f</td>\n", results[i].duration_ms);
        fprintf(f, "</tr>\n");
        
        if (results[i].failed > 0 && strlen(results[i].error_details) > 0) {
            fprintf(f, "<tr><td colspan='5'>\n");
            fprintf(f, "<div class='error-details'>Error Details:<br>%s</div>\n", results[i].error_details);
            fprintf(f, "</td></tr>\n");
        }
    }
    
    fprintf(f, "</table>\n");
    fprintf(f, "</body>\n</html>\n");
    fclose(f);
    
    printf("📊 HTML report generated: %s\n", filepath);
}

static void generate_junit_xml(TestSuiteResult* results, int count) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/junit-results.xml", config.output_dir);
    
    FILE* f = fopen(filepath, "w");
    if (!f) {
        printf("⚠️  Could not create JUnit XML: %s\n", filepath);
        return;
    }
    
    int total_tests = 0, total_failures = 0;
    double total_time = 0.0;
    
    for (int i = 0; i < count; i++) {
        total_tests += results[i].total;
        total_failures += results[i].failed;
        total_time += results[i].duration_ms;
    }
    
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<testsuites name=\"SIGNAL Tests\" tests=\"%d\" failures=\"%d\" time=\"%.3f\">\n", 
            total_tests, total_failures, total_time / 1000.0);
    
    for (int i = 0; i < count; i++) {
        const char* suite_name = strrchr(results[i].name, '/');
        suite_name = suite_name ? suite_name + 1 : results[i].name;
        
        fprintf(f, "  <testsuite name=\"%s\" tests=\"%d\" failures=\"%d\" time=\"%.3f\">\n",
                suite_name, results[i].total, results[i].failed, results[i].duration_ms / 1000.0);
        
        // Individual test cases (simplified - would need more detailed parsing for real cases)
        for (int j = 0; j < results[i].total; j++) {
            bool is_failure = j < results[i].failed;
            fprintf(f, "    <testcase name=\"test_%d\" classname=\"%s\" time=\"%.3f\"",
                    j + 1, suite_name, (results[i].duration_ms / results[i].total) / 1000.0);
            
            if (is_failure) {
                fprintf(f, ">\n");
                fprintf(f, "      <failure message=\"Test failed\">%s</failure>\n", results[i].error_details);
                fprintf(f, "    </testcase>\n");
            } else {
                fprintf(f, " />\n");
            }
        }
        
        fprintf(f, "  </testsuite>\n");
    }
    
    fprintf(f, "</testsuites>\n");
    fclose(f);
    
    printf("📊 JUnit XML generated: %s\n", filepath);
}

static void generate_coverage_report(void) {
    if (!config.coverage) return;
    
    printf("📊 Generating coverage report...\n");
    
    // Run gcov to generate coverage data
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "cd %s && find . -name '*.gcda' -exec gcov {} \\;", config.output_dir);
    system(cmd);
    
    // Check if lcov is available for HTML coverage reports
    if (system("which lcov > /dev/null 2>&1") == 0) {
        printf("📊 Generating HTML coverage report with lcov...\n");
        
        // Generate lcov info file
        snprintf(cmd, sizeof(cmd), 
                "cd %s && lcov --capture --directory ../build --output-file coverage.info", 
                config.output_dir);
        system(cmd);
        
        // Generate HTML report
        snprintf(cmd, sizeof(cmd), 
                "cd %s && genhtml coverage.info --output-directory coverage_html", 
                config.output_dir);
        system(cmd);
        
        printf("📊 Coverage HTML report: %s/coverage_html/index.html\n", config.output_dir);
    } else {
        printf("⚠️  lcov not found - install it for HTML coverage reports\n");
        printf("📊 Coverage .gcov files generated in %s\n", config.output_dir);
    }
}

// Main execution
static void print_usage(const char* program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  -v, --verbose       Verbose output\n");
    printf("  -h, --html          Generate HTML report\n");
    printf("  -x, --xml           Generate JUnit XML\n");
    printf("  -c, --coverage      Generate coverage report\n");
    printf("  -f, --filter PATTERN Run only suites matching pattern\n");
    printf("  -o, --output DIR    Output directory (default: test_results)\n");
    printf("  --help              Show this help\n");
}

static void parse_arguments(int argc, char* argv[]) {
    // Default configuration
    config.verbose = false;
    config.html_report = false;
    config.junit_xml = false;
    config.coverage = false;
    config.filter_enabled = false;
    strcpy(config.output_dir, "test_results");
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            config.verbose = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--html") == 0) {
            config.html_report = true;
        } else if (strcmp(argv[i], "-x") == 0 || strcmp(argv[i], "--xml") == 0) {
            config.junit_xml = true;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--coverage") == 0) {
            config.coverage = true;
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--filter") == 0) {
            if (i + 1 < argc) {
                config.filter_enabled = true;
                strcpy(config.filter_pattern, argv[++i]);
            } else {
                printf("Error: --filter requires a pattern\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                strcpy(config.output_dir, argv[++i]);
            } else {
                printf("Error: --output requires a directory\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            exit(0);
        } else {
            printf("Error: Unknown option %s\n", argv[i]);
            print_usage(argv[0]);
            exit(1);
        }
    }
}

int main(int argc, char* argv[]) {
    parse_arguments(argc, argv);
    
    printf("🚀 SIGNAL Test Runner\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    if (config.filter_enabled) {
        printf("🔍 Filter: %s\n", config.filter_pattern);
    }
    
    // Create output directory
    create_directory(config.output_dir);
    
    // Run all test suites
    int results_count = 0;
    int total_failures = 0;
    
    for (int i = 0; i < num_test_suites; i++) {
        if (!should_run_suite(test_suites[i])) {
            continue;
        }
        
        TestSuiteResult* result = &results[results_count++];
        run_test_suite(test_suites[i], result);
        
        if (result->failed > 0) {
            total_failures += result->failed;
        }
    }
    
    // Generate reports
    generate_console_report(results, results_count);
    
    if (config.html_report) {
        generate_html_report(results, results_count);
    }
    
    if (config.junit_xml) {
        generate_junit_xml(results, results_count);
    }
    
    if (config.coverage) {
        generate_coverage_report();
    }
    
    printf("\n📁 Results saved to: %s\n", config.output_dir);
    
    return total_failures > 0 ? 1 : 0;
}
