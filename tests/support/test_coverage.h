/**
 * @file test_coverage.h
 * @brief Test coverage analytics and reporting system
 * 
 * This system provides comprehensive test coverage analysis using cloc integration
 * and custom analytics to ensure quality gates are met across all system modules.
 */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// ============================================================================
// COVERAGE ANALYTICS STRUCTURES
// ============================================================================

/**
 * @brief Coverage analysis for a single source file
 */
typedef struct {
    char file_path[256];           // Path to source file
    char module_name[64];          // Module/system name
    uint32_t total_lines;          // Total lines of code
    uint32_t comment_lines;        // Comment lines
    uint32_t blank_lines;          // Blank lines
    uint32_t code_lines;           // Actual code lines
    uint32_t tested_lines;         // Lines covered by tests
    uint32_t untested_lines;       // Lines not covered by tests
    double coverage_percentage;    // Coverage percentage (0-100)
    bool meets_target;             // Whether coverage meets target
    double target_percentage;      // Target coverage percentage
} FileCoverage;

/**
 * @brief Coverage analysis for a module/system
 */
typedef struct {
    char module_name[64];          // Module/system name
    FileCoverage* files;           // Array of file coverage data
    uint32_t file_count;           // Number of files in module
    uint32_t total_code_lines;     // Total code lines in module
    uint32_t total_tested_lines;   // Total tested lines in module
    double coverage_percentage;    // Module coverage percentage
    bool meets_target;             // Whether module meets target
    double target_percentage;      // Target coverage percentage
    uint32_t test_count;           // Number of tests for this module
} ModuleCoverage;

/**
 * @brief Overall project coverage statistics
 */
typedef struct {
    ModuleCoverage* modules;       // Array of module coverage data
    uint32_t module_count;         // Number of modules
    uint32_t total_source_files;   // Total source files analyzed
    uint32_t total_test_files;     // Total test files
    uint32_t total_code_lines;     // Total code lines in project
    uint32_t total_tested_lines;   // Total tested lines in project
    double overall_coverage;       // Overall coverage percentage
    bool meets_overall_target;     // Whether project meets overall target
    double overall_target;         // Overall target coverage percentage
    time_t analysis_timestamp;     // When analysis was performed
    char git_commit[41];           // Git commit hash (40 chars + null)
} ProjectCoverage;

/**
 * @brief Coverage trend data point
 */
typedef struct {
    time_t timestamp;              // When measurement was taken
    double coverage_percentage;    // Coverage at this time
    uint32_t total_tests;          // Number of tests at this time
    uint32_t total_code_lines;     // Lines of code at this time
    char git_commit[41];           // Git commit hash
} CoverageTrendPoint;

/**
 * @brief Coverage trend analysis
 */
typedef struct {
    CoverageTrendPoint* data_points;  // Array of trend data
    uint32_t point_count;             // Number of data points
    uint32_t capacity;                // Capacity of data points array
    double trend_slope;               // Trend slope (positive = improving)
    bool is_improving;                // Whether coverage is trending up
} CoverageTrend;

/**
 * @brief Coverage target configuration
 */
typedef struct {
    double core_systems_target;       // Core ECS systems target (default: 95%)
    double physics_systems_target;    // Physics systems target (default: 90%)
    double rendering_systems_target;  // Rendering systems target (default: 85%)
    double ui_systems_target;         // UI systems target (default: 80%)
    double integration_target;        // Integration tests target (default: 100%)
    double overall_project_target;    // Overall project target (default: 85%)
} CoverageTargets;

// ============================================================================
// COVERAGE ANALYTICS API
// ============================================================================

/**
 * @brief Initialize the coverage analytics system
 * @return true if initialization successful, false otherwise
 */
bool coverage_analytics_init(void);

/**
 * @brief Shutdown the coverage analytics system
 */
void coverage_analytics_shutdown(void);

/**
 * @brief Set coverage targets for different module types
 * @param targets Coverage target configuration
 * @return true if successful, false otherwise
 */
bool coverage_set_targets(const CoverageTargets* targets);

/**
 * @brief Get current coverage targets
 * @param out_targets Pointer to receive current targets
 * @return true if successful, false otherwise
 */
bool coverage_get_targets(CoverageTargets* out_targets);

/**
 * @brief Analyze coverage for the entire project
 * @param source_directory Path to source code directory
 * @param test_directory Path to test directory
 * @param out_coverage Pointer to receive coverage analysis
 * @return true if successful, false otherwise
 */
bool coverage_analyze_project(const char* source_directory,
                             const char* test_directory,
                             ProjectCoverage* out_coverage);

/**
 * @brief Analyze coverage for a specific module
 * @param module_name Name of module to analyze
 * @param source_path Path to module source files
 * @param test_path Path to module test files
 * @param out_coverage Pointer to receive module coverage
 * @return true if successful, false otherwise
 */
bool coverage_analyze_module(const char* module_name,
                            const char* source_path,
                            const char* test_path,
                            ModuleCoverage* out_coverage);

/**
 * @brief Analyze coverage for a specific file
 * @param file_path Path to source file
 * @param module_name Module name (for categorization)
 * @param out_coverage Pointer to receive file coverage
 * @return true if successful, false otherwise
 */
bool coverage_analyze_file(const char* file_path,
                          const char* module_name,
                          FileCoverage* out_coverage);

/**
 * @brief Record coverage measurement for trend tracking
 * @param coverage Current project coverage data
 * @return true if successful, false otherwise
 */
bool coverage_record_trend_point(const ProjectCoverage* coverage);

/**
 * @brief Get coverage trend analysis
 * @param days_back Number of days of history to analyze
 * @param out_trend Pointer to receive trend analysis
 * @return true if successful, false otherwise
 */
bool coverage_get_trend(uint32_t days_back, CoverageTrend* out_trend);

/**
 * @brief Check if coverage targets are met
 * @param coverage Project coverage data to check
 * @param out_failing_modules Array to receive failing module names (can be NULL)
 * @param max_failing Maximum number of failing modules to return
 * @return Number of modules failing to meet targets
 */
uint32_t coverage_check_targets(const ProjectCoverage* coverage,
                               char (*out_failing_modules)[64],
                               uint32_t max_failing);

/**
 * @brief Generate coverage report in various formats
 * @param coverage Project coverage data
 * @param format Report format ("text", "json", "html", "xml")
 * @param output_file Path to output file (NULL for stdout)
 * @return true if successful, false otherwise
 */
bool coverage_generate_report(const ProjectCoverage* coverage,
                             const char* format,
                             const char* output_file);

/**
 * @brief Generate coverage trend report
 * @param trend Coverage trend data
 * @param format Report format ("text", "json", "html")
 * @param output_file Path to output file (NULL for stdout)
 * @return true if successful, false otherwise
 */
bool coverage_generate_trend_report(const CoverageTrend* trend,
                                   const char* format,
                                   const char* output_file);

/**
 * @brief Generate coverage gap report (showing untested code)
 * @param coverage Project coverage data
 * @param min_priority_level Minimum severity level to include
 * @param output_file Path to output file (NULL for stdout)
 * @return true if successful, false otherwise
 */
bool coverage_generate_gap_report(const ProjectCoverage* coverage,
                                 const char* min_priority_level,
                                 const char* output_file);

// ============================================================================
// CLOC INTEGRATION
// ============================================================================

/**
 * @brief Execute cloc analysis on directory
 * @param directory Path to directory to analyze
 * @param include_pattern File pattern to include (can be NULL)
 * @param exclude_pattern File pattern to exclude (can be NULL)
 * @param output_file Path to output CSV file
 * @return true if successful, false otherwise
 */
bool cloc_analyze_directory(const char* directory,
                           const char* include_pattern,
                           const char* exclude_pattern,
                           const char* output_file);

/**
 * @brief Parse cloc CSV output into coverage structures
 * @param cloc_csv_file Path to cloc CSV output file
 * @param out_files Array to receive file coverage data
 * @param max_files Maximum number of files to parse
 * @return Number of files parsed
 */
uint32_t cloc_parse_csv(const char* cloc_csv_file,
                       FileCoverage* out_files,
                       uint32_t max_files);

/**
 * @brief Get cloc statistics summary
 * @param directory Path to directory to analyze
 * @param out_total_files Pointer to receive total file count
 * @param out_total_lines Pointer to receive total line count
 * @param out_code_lines Pointer to receive code line count
 * @param out_comment_lines Pointer to receive comment line count
 * @return true if successful, false otherwise
 */
bool cloc_get_summary(const char* directory,
                     uint32_t* out_total_files,
                     uint32_t* out_total_lines,
                     uint32_t* out_code_lines,
                     uint32_t* out_comment_lines);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Free project coverage data
 * @param coverage Project coverage data to free
 */
void coverage_free_project_data(ProjectCoverage* coverage);

/**
 * @brief Free module coverage data
 * @param coverage Module coverage data to free
 */
void coverage_free_module_data(ModuleCoverage* coverage);

/**
 * @brief Free coverage trend data
 * @param trend Coverage trend data to free
 */
void coverage_free_trend_data(CoverageTrend* trend);

/**
 * @brief Calculate coverage percentage
 * @param tested_lines Number of tested lines
 * @param total_lines Total number of lines
 * @return Coverage percentage (0-100)
 */
double coverage_calculate_percentage(uint32_t tested_lines, uint32_t total_lines);

/**
 * @brief Determine module type from path/name
 * @param module_name Module name or path
 * @return Module type string ("core", "physics", "rendering", "ui", "other")
 */
const char* coverage_determine_module_type(const char* module_name);

/**
 * @brief Get target coverage for module type
 * @param module_type Module type string
 * @param targets Coverage targets configuration
 * @return Target coverage percentage for module type
 */
double coverage_get_target_for_module_type(const char* module_type,
                                          const CoverageTargets* targets);

/**
 * @brief Print coverage summary to console
 * @param coverage Project coverage data
 */
void coverage_print_summary(const ProjectCoverage* coverage);

/**
 * @brief Print coverage trend summary to console
 * @param trend Coverage trend data
 */
void coverage_print_trend_summary(const CoverageTrend* trend);

/**
 * @brief Export coverage data to JSON
 * @param coverage Project coverage data
 * @param output_file Path to JSON output file
 * @return true if successful, false otherwise
 */
bool coverage_export_json(const ProjectCoverage* coverage, const char* output_file);

/**
 * @brief Import coverage data from JSON
 * @param input_file Path to JSON input file
 * @param out_coverage Pointer to receive imported coverage data
 * @return true if successful, false otherwise
 */
bool coverage_import_json(const char* input_file, ProjectCoverage* out_coverage);

// ============================================================================
// AUTOMATED COVERAGE MONITORING
// ============================================================================

/**
 * @brief Start automated coverage monitoring
 * @param check_interval_minutes Interval between checks in minutes
 * @param alert_threshold_drop Alert if coverage drops by this percentage
 * @return true if monitoring started, false otherwise
 */
bool coverage_start_monitoring(uint32_t check_interval_minutes,
                              double alert_threshold_drop);

/**
 * @brief Stop automated coverage monitoring
 */
void coverage_stop_monitoring(void);

/**
 * @brief Check if coverage monitoring is active
 * @return true if monitoring is active, false otherwise
 */
bool coverage_is_monitoring_active(void);

/**
 * @brief Get coverage monitoring status
 * @param out_last_check Pointer to receive last check timestamp
 * @param out_next_check Pointer to receive next check timestamp
 * @param out_alert_count Pointer to receive alert count
 * @return true if successful, false otherwise
 */
bool coverage_get_monitoring_status(time_t* out_last_check,
                                   time_t* out_next_check,
                                   uint32_t* out_alert_count);

#endif // TEST_COVERAGE_H
