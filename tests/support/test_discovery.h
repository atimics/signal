/**
 * @file test_discovery.h
 * @brief Automated test discovery and registration system
 * 
 * This system automatically discovers and registers test functions based on
 * naming conventions and annotations, supporting the comprehensive test
 * organization outlined in TEST_SUITE_ORGANIZATION.md
 */

#ifndef TEST_DISCOVERY_H
#define TEST_DISCOVERY_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
struct TestRunner;
struct TestSuite;

// ============================================================================
// TEST DISCOVERY SYSTEM
// ============================================================================

/**
 * @brief Test categories for automated discovery and organization
 */
typedef enum {
    TEST_CATEGORY_UNIT = 0,        // Pure unit tests (no dependencies)
    TEST_CATEGORY_INTEGRATION,     // Cross-system integration tests
    TEST_CATEGORY_PERFORMANCE,     // Benchmarks and performance tests
    TEST_CATEGORY_REGRESSION,      // Specific bug regression tests
    TEST_CATEGORY_SMOKE,          // Critical path smoke tests
    TEST_CATEGORY_ACCEPTANCE,     // User acceptance tests
    TEST_CATEGORY_EXPERIMENTAL,   // Development/experimental tests
    TEST_CATEGORY_COUNT
} TestCategory;

/**
 * @brief Test priority levels for execution ordering
 */
typedef enum {
    TEST_PRIORITY_CRITICAL = 0,    // Must pass for build to succeed
    TEST_PRIORITY_HIGH,            // Important functionality tests
    TEST_PRIORITY_MEDIUM,          // Standard functionality tests
    TEST_PRIORITY_LOW,             // Nice-to-have tests
    TEST_PRIORITY_COUNT
} TestPriority;

/**
 * @brief Test function signature
 */
typedef void (*TestFunction)(void);

/**
 * @brief Test suite registration function signature
 */
typedef void (*TestSuiteFunction)(void);

/**
 * @brief Test metadata structure
 */
typedef struct {
    const char* name;              // Test function name
    const char* description;       // Human-readable description
    const char* file;              // Source file path
    int line;                      // Line number in source
    TestFunction function;         // Test function pointer
    TestCategory category;         // Test category
    TestPriority priority;         // Test priority
    const char* tags;              // Comma-separated tags
    double timeout_seconds;        // Maximum execution time
    bool enabled;                  // Whether test is enabled
} TestMetadata;

/**
 * @brief Test suite metadata structure
 */
typedef struct {
    const char* name;              // Suite name
    const char* description;       // Suite description
    const char* module;            // Module/system name
    TestSuiteFunction function;    // Suite registration function
    TestMetadata* tests;           // Array of tests in suite
    uint32_t test_count;           // Number of tests in suite
    bool enabled;                  // Whether suite is enabled
} TestSuiteMetadata;

/**
 * @brief Test discovery statistics
 */
typedef struct {
    uint32_t total_tests;          // Total number of tests discovered
    uint32_t total_suites;         // Total number of suites discovered
    uint32_t tests_by_category[TEST_CATEGORY_COUNT];  // Tests per category
    uint32_t tests_by_priority[TEST_PRIORITY_COUNT];  // Tests per priority
    uint32_t enabled_tests;        // Number of enabled tests
    uint32_t disabled_tests;       // Number of disabled tests
} TestDiscoveryStats;

// ============================================================================
// TEST DISCOVERY API
// ============================================================================

/**
 * @brief Initialize the test discovery system
 * @return true if initialization successful, false otherwise
 */
bool test_discovery_init(void);

/**
 * @brief Shutdown the test discovery system
 */
void test_discovery_shutdown(void);

/**
 * @brief Discover all tests in the specified directory
 * @param test_directory Path to test directory to scan
 * @return true if discovery successful, false otherwise
 */
bool test_discovery_scan_directory(const char* test_directory);

/**
 * @brief Register a test function manually
 * @param metadata Test metadata structure
 * @return true if registration successful, false otherwise
 */
bool test_discovery_register_test(const TestMetadata* metadata);

/**
 * @brief Register a test suite manually
 * @param suite_metadata Test suite metadata structure
 * @return true if registration successful, false otherwise
 */
bool test_discovery_register_suite(const TestSuiteMetadata* suite_metadata);

/**
 * @brief Get all discovered tests
 * @param out_tests Pointer to receive array of test metadata
 * @param out_count Pointer to receive test count
 * @return true if successful, false otherwise
 */
bool test_discovery_get_tests(TestMetadata** out_tests, uint32_t* out_count);

/**
 * @brief Get all discovered test suites
 * @param out_suites Pointer to receive array of suite metadata
 * @param out_count Pointer to receive suite count
 * @return true if successful, false otherwise
 */
bool test_discovery_get_suites(TestSuiteMetadata** out_suites, uint32_t* out_count);

/**
 * @brief Get tests by category
 * @param category Test category to filter by
 * @param out_tests Pointer to receive array of test metadata
 * @param out_count Pointer to receive test count
 * @return true if successful, false otherwise
 */
bool test_discovery_get_tests_by_category(TestCategory category, 
                                         TestMetadata** out_tests, 
                                         uint32_t* out_count);

/**
 * @brief Get tests by priority
 * @param priority Test priority to filter by
 * @param out_tests Pointer to receive array of test metadata
 * @param out_count Pointer to receive test count
 * @return true if successful, false otherwise
 */
bool test_discovery_get_tests_by_priority(TestPriority priority,
                                         TestMetadata** out_tests,
                                         uint32_t* out_count);

/**
 * @brief Get tests by tag
 * @param tag Tag to filter by
 * @param out_tests Pointer to receive array of test metadata
 * @param out_count Pointer to receive test count
 * @return true if successful, false otherwise
 */
bool test_discovery_get_tests_by_tag(const char* tag,
                                    TestMetadata** out_tests,
                                    uint32_t* out_count);

/**
 * @brief Get discovery statistics
 * @param out_stats Pointer to receive statistics structure
 * @return true if successful, false otherwise
 */
bool test_discovery_get_stats(TestDiscoveryStats* out_stats);

/**
 * @brief Enable or disable a specific test
 * @param test_name Name of test to enable/disable
 * @param enabled Whether to enable or disable the test
 * @return true if successful, false otherwise
 */
bool test_discovery_set_test_enabled(const char* test_name, bool enabled);

/**
 * @brief Enable or disable a test suite
 * @param suite_name Name of suite to enable/disable
 * @param enabled Whether to enable or disable the suite
 * @return true if successful, false otherwise
 */
bool test_discovery_set_suite_enabled(const char* suite_name, bool enabled);

// ============================================================================
// TEST REGISTRATION MACROS
// ============================================================================

/**
 * @brief Register a test function with full metadata
 * @param func_name Test function name
 * @param desc Description of the test
 * @param cat Test category
 * @param pri Test priority
 * @param tag_list Comma-separated list of tags
 * @param timeout_sec Maximum execution time in seconds
 */
#define REGISTER_TEST_FULL(func_name, desc, cat, pri, tag_list, timeout_sec) \
    static void __attribute__((constructor)) register_##func_name(void) { \
        TestMetadata metadata = { \
            .name = #func_name, \
            .description = desc, \
            .file = __FILE__, \
            .line = __LINE__, \
            .function = func_name, \
            .category = cat, \
            .priority = pri, \
            .tags = tag_list, \
            .timeout_seconds = timeout_sec, \
            .enabled = true \
        }; \
        test_discovery_register_test(&metadata); \
    }

/**
 * @brief Register a test function with standard metadata
 * @param func_name Test function name
 * @param desc Description of the test
 * @param cat Test category
 */
#define REGISTER_TEST(func_name, desc, cat) \
    REGISTER_TEST_FULL(func_name, desc, cat, TEST_PRIORITY_MEDIUM, "", 30.0)

/**
 * @brief Register a critical test function
 * @param func_name Test function name
 * @param desc Description of the test
 */
#define REGISTER_CRITICAL_TEST(func_name, desc) \
    REGISTER_TEST_FULL(func_name, desc, TEST_CATEGORY_UNIT, TEST_PRIORITY_CRITICAL, "critical", 10.0)

/**
 * @brief Register a performance test function
 * @param func_name Test function name
 * @param desc Description of the test
 * @param timeout_sec Maximum execution time in seconds
 */
#define REGISTER_PERFORMANCE_TEST(func_name, desc, timeout_sec) \
    REGISTER_TEST_FULL(func_name, desc, TEST_CATEGORY_PERFORMANCE, TEST_PRIORITY_MEDIUM, "performance", timeout_sec)

/**
 * @brief Register a test suite
 * @param suite_name Suite function name
 * @param desc Description of the suite
 * @param mod Module/system name
 */
#define REGISTER_TEST_SUITE(suite_name, desc, mod) \
    static void __attribute__((constructor)) register_suite_##suite_name(void) { \
        TestSuiteMetadata metadata = { \
            .name = #suite_name, \
            .description = desc, \
            .module = mod, \
            .function = suite_name, \
            .tests = NULL, \
            .test_count = 0, \
            .enabled = true \
        }; \
        test_discovery_register_suite(&metadata); \
    }

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Convert test category to string
 * @param category Test category
 * @return String representation of category
 */
const char* test_category_to_string(TestCategory category);

/**
 * @brief Convert test priority to string
 * @param priority Test priority
 * @return String representation of priority
 */
const char* test_priority_to_string(TestPriority priority);

/**
 * @brief Parse test category from string
 * @param category_str String representation of category
 * @return Test category enum value
 */
TestCategory test_category_from_string(const char* category_str);

/**
 * @brief Parse test priority from string
 * @param priority_str String representation of priority
 * @return Test priority enum value
 */
TestPriority test_priority_from_string(const char* priority_str);

/**
 * @brief Check if test has a specific tag
 * @param test Test metadata
 * @param tag Tag to check for
 * @return true if test has the tag, false otherwise
 */
bool test_has_tag(const TestMetadata* test, const char* tag);

/**
 * @brief Print discovery statistics to console
 * @param stats Discovery statistics
 */
void test_discovery_print_stats(const TestDiscoveryStats* stats);

/**
 * @brief Generate test discovery report
 * @param output_file Path to output file (NULL for stdout)
 * @return true if successful, false otherwise
 */
bool test_discovery_generate_report(const char* output_file);

#endif // TEST_DISCOVERY_H
