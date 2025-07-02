/**
 * @file test_runner.h
 * @brief Unified test runner with advanced features
 * 
 * This system provides a comprehensive test execution framework with support
 * for parallel execution, filtering, reporting, and integration with our
 * test discovery and coverage systems.
 */

#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "test_discovery.h"
#include "test_coverage.h"

// ============================================================================
// TEST EXECUTION STRUCTURES
// ============================================================================

/**
 * @brief Test execution result
 */
typedef enum {
    TEST_RESULT_PASS = 0,          // Test passed
    TEST_RESULT_FAIL,              // Test failed
    TEST_RESULT_SKIP,              // Test was skipped
    TEST_RESULT_TIMEOUT,           // Test exceeded timeout
    TEST_RESULT_ERROR,             // Test had execution error
    TEST_RESULT_COUNT
} TestResult;

/**
 * @brief Test execution context
 */
typedef struct {
    TestMetadata* test;            // Test metadata
    TestResult result;             // Execution result
    double execution_time;         // Execution time in seconds
    char error_message[512];       // Error message if failed
    uint32_t assertions_passed;    // Number of assertions that passed
    uint32_t assertions_failed;    // Number of assertions that failed
    time_t start_time;             // Test start time
    time_t end_time;               // Test end time
    bool timed_out;                // Whether test timed out
} TestExecution;

/**
 * @brief Test suite execution result
 */
typedef struct {
    TestSuiteMetadata* suite;      // Suite metadata
    TestExecution* test_results;   // Array of test execution results
    uint32_t test_count;           // Number of tests executed
    uint32_t passed_count;         // Number of tests that passed
    uint32_t failed_count;         // Number of tests that failed
    uint32_t skipped_count;        // Number of tests that were skipped
    uint32_t timeout_count;        // Number of tests that timed out
    uint32_t error_count;          // Number of tests with errors
    double total_execution_time;   // Total suite execution time
    bool suite_passed;             // Whether entire suite passed
} SuiteExecution;

/**
 * @brief Test run configuration
 */
typedef struct {
    // Execution control
    bool parallel_execution;       // Enable parallel test execution
    uint32_t max_parallel_tests;   // Maximum number of parallel tests
    double default_timeout;        // Default test timeout in seconds
    bool fail_fast;                // Stop on first failure
    bool continue_on_error;        // Continue execution after errors
    
    // Filtering options
    TestCategory* categories;      // Categories to include (NULL for all)
    uint32_t category_count;       // Number of categories to include
    TestPriority min_priority;     // Minimum priority to execute
    char* tag_filter;              // Tag filter expression
    char* name_pattern;            // Test name pattern filter
    
    // Output control
    bool verbose_output;           // Enable verbose output
    bool quiet_mode;               // Suppress non-essential output
    bool show_progress;            // Show progress indicator
    bool color_output;             // Enable colored output
    
    // Reporting options
    bool generate_junit_xml;       // Generate JUnit XML report
    bool generate_coverage_report; // Generate coverage report
    bool generate_performance_report; // Generate performance report
    char output_directory[256];    // Output directory for reports
    
    // Advanced options
    bool shuffle_tests;            // Randomize test execution order
    uint32_t random_seed;          // Random seed for shuffling
    uint32_t repeat_count;         // Number of times to repeat tests
    bool stress_test_mode;         // Enable stress testing mode
} TestRunConfig;

/**
 * @brief Test run statistics
 */
typedef struct {
    uint32_t total_tests;          // Total number of tests
    uint32_t executed_tests;       // Number of tests executed
    uint32_t passed_tests;         // Number of tests that passed
    uint32_t failed_tests;         // Number of tests that failed
    uint32_t skipped_tests;        // Number of tests that were skipped
    uint32_t timeout_tests;        // Number of tests that timed out
    uint32_t error_tests;          // Number of tests with errors
    double total_execution_time;   // Total execution time
    double average_test_time;      // Average test execution time
    double fastest_test_time;      // Fastest test execution time
    double slowest_test_time;      // Slowest test execution time
    bool all_passed;               // Whether all tests passed
    time_t run_start_time;         // Test run start time
    time_t run_end_time;           // Test run end time
} TestRunStats;

/**
 * @brief Test runner progress callback
 */
typedef void (*TestProgressCallback)(const TestExecution* execution, void* user_data);

/**
 * @brief Test runner completion callback
 */
typedef void (*TestCompletionCallback)(const TestRunStats* stats, void* user_data);

// ============================================================================
// TEST RUNNER API
// ============================================================================

/**
 * @brief Initialize the test runner system
 * @return true if initialization successful, false otherwise
 */
bool test_runner_init(void);

/**
 * @brief Shutdown the test runner system
 */
void test_runner_shutdown(void);

/**
 * @brief Set default test run configuration
 * @param config Configuration to set as default
 * @return true if successful, false otherwise
 */
bool test_runner_set_default_config(const TestRunConfig* config);

/**
 * @brief Get current default configuration
 * @param out_config Pointer to receive current configuration
 * @return true if successful, false otherwise
 */
bool test_runner_get_default_config(TestRunConfig* out_config);

/**
 * @brief Run all discovered tests with default configuration
 * @param out_stats Pointer to receive run statistics (can be NULL)
 * @return true if all tests passed, false otherwise
 */
bool test_runner_run_all(TestRunStats* out_stats);

/**
 * @brief Run tests with custom configuration
 * @param config Test run configuration
 * @param out_stats Pointer to receive run statistics (can be NULL)
 * @return true if all tests passed, false otherwise
 */
bool test_runner_run_with_config(const TestRunConfig* config, TestRunStats* out_stats);

/**
 * @brief Run tests by category
 * @param category Test category to run
 * @param out_stats Pointer to receive run statistics (can be NULL)
 * @return true if all tests passed, false otherwise
 */
bool test_runner_run_category(TestCategory category, TestRunStats* out_stats);

/**
 * @brief Run tests by priority
 * @param min_priority Minimum priority to run
 * @param out_stats Pointer to receive run statistics (can be NULL)
 * @return true if all tests passed, false otherwise
 */
bool test_runner_run_priority(TestPriority min_priority, TestRunStats* out_stats);

/**
 * @brief Run tests by tag
 * @param tag Tag to filter by
 * @param out_stats Pointer to receive run statistics (can be NULL)
 * @return true if all tests passed, false otherwise
 */
bool test_runner_run_tag(const char* tag, TestRunStats* out_stats);

/**
 * @brief Run a specific test suite
 * @param suite_name Name of suite to run
 * @param out_execution Pointer to receive suite execution result (can be NULL)
 * @return true if suite passed, false otherwise
 */
bool test_runner_run_suite(const char* suite_name, SuiteExecution* out_execution);

/**
 * @brief Run a specific test
 * @param test_name Name of test to run
 * @param out_execution Pointer to receive test execution result (can be NULL)
 * @return true if test passed, false otherwise
 */
bool test_runner_run_test(const char* test_name, TestExecution* out_execution);

/**
 * @brief Set progress callback
 * @param callback Progress callback function
 * @param user_data User data to pass to callback
 */
void test_runner_set_progress_callback(TestProgressCallback callback, void* user_data);

/**
 * @brief Set completion callback
 * @param callback Completion callback function
 * @param user_data User data to pass to callback
 */
void test_runner_set_completion_callback(TestCompletionCallback callback, void* user_data);

/**
 * @brief Abort currently running test execution
 * @return true if successful, false otherwise
 */
bool test_runner_abort(void);

/**
 * @brief Check if test runner is currently executing
 * @return true if executing, false otherwise
 */
bool test_runner_is_running(void);

/**
 * @brief Get current execution statistics
 * @param out_stats Pointer to receive current statistics
 * @return true if successful, false otherwise
 */
bool test_runner_get_current_stats(TestRunStats* out_stats);

// ============================================================================
// REPORTING FUNCTIONS
// ============================================================================

/**
 * @brief Generate test execution report
 * @param stats Test run statistics
 * @param format Report format ("text", "json", "html", "junit")
 * @param output_file Path to output file (NULL for stdout)
 * @return true if successful, false otherwise
 */
bool test_runner_generate_report(const TestRunStats* stats,
                                const char* format,
                                const char* output_file);

/**
 * @brief Generate JUnit XML report
 * @param stats Test run statistics
 * @param suite_results Array of suite execution results
 * @param suite_count Number of suite results
 * @param output_file Path to output XML file
 * @return true if successful, false otherwise
 */
bool test_runner_generate_junit_xml(const TestRunStats* stats,
                                   const SuiteExecution* suite_results,
                                   uint32_t suite_count,
                                   const char* output_file);

/**
 * @brief Generate performance report
 * @param stats Test run statistics
 * @param test_results Array of test execution results
 * @param test_count Number of test results
 * @param output_file Path to output file (NULL for stdout)
 * @return true if successful, false otherwise
 */
bool test_runner_generate_performance_report(const TestRunStats* stats,
                                            const TestExecution* test_results,
                                            uint32_t test_count,
                                            const char* output_file);

/**
 * @brief Generate failure analysis report
 * @param failed_tests Array of failed test executions
 * @param failed_count Number of failed tests
 * @param output_file Path to output file (NULL for stdout)
 * @return true if successful, false otherwise
 */
bool test_runner_generate_failure_report(const TestExecution* failed_tests,
                                         uint32_t failed_count,
                                         const char* output_file);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Convert test result to string
 * @param result Test result enum
 * @return String representation of result
 */
const char* test_result_to_string(TestResult result);

/**
 * @brief Get result color code for terminal output
 * @param result Test result enum
 * @return ANSI color code string
 */
const char* test_result_color_code(TestResult result);

/**
 * @brief Print test run statistics to console
 * @param stats Test run statistics
 */
void test_runner_print_stats(const TestRunStats* stats);

/**
 * @brief Print test execution result to console
 * @param execution Test execution result
 * @param verbose Whether to show verbose output
 */
void test_runner_print_result(const TestExecution* execution, bool verbose);

/**
 * @brief Calculate test run summary
 * @param executions Array of test executions
 * @param count Number of executions
 * @param out_stats Pointer to receive calculated statistics
 */
void test_runner_calculate_stats(const TestExecution* executions,
                                uint32_t count,
                                TestRunStats* out_stats);

/**
 * @brief Free test execution results
 * @param executions Array of test executions to free
 * @param count Number of executions
 */
void test_runner_free_executions(TestExecution* executions, uint32_t count);

/**
 * @brief Free suite execution results
 * @param suite_executions Array of suite executions to free
 * @param count Number of suite executions
 */
void test_runner_free_suite_executions(SuiteExecution* suite_executions, uint32_t count);

// ============================================================================
// COMMAND LINE INTERFACE
// ============================================================================

/**
 * @brief Parse command line arguments into test run configuration
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @param out_config Pointer to receive parsed configuration
 * @return true if parsing successful, false otherwise
 */
bool test_runner_parse_args(int argc, char** argv, TestRunConfig* out_config);

/**
 * @brief Print command line usage information
 * @param program_name Name of the program
 */
void test_runner_print_usage(const char* program_name);

/**
 * @brief Main test runner entry point for command line execution
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return Exit code (0 for success, non-zero for failure)
 */
int test_runner_main(int argc, char** argv);

// ============================================================================
// INTEGRATION WITH COVERAGE SYSTEM
// ============================================================================

/**
 * @brief Run tests with coverage analysis
 * @param config Test run configuration
 * @param source_directory Path to source code directory
 * @param out_stats Pointer to receive test statistics (can be NULL)
 * @param out_coverage Pointer to receive coverage analysis (can be NULL)
 * @return true if all tests passed and coverage targets met, false otherwise
 */
bool test_runner_run_with_coverage(const TestRunConfig* config,
                                  const char* source_directory,
                                  TestRunStats* out_stats,
                                  ProjectCoverage* out_coverage);

/**
 * @brief Generate combined test and coverage report
 * @param stats Test run statistics
 * @param coverage Coverage analysis results
 * @param output_file Path to output file (NULL for stdout)
 * @return true if successful, false otherwise
 */
bool test_runner_generate_combined_report(const TestRunStats* stats,
                                         const ProjectCoverage* coverage,
                                         const char* output_file);

#endif // TEST_RUNNER_H
