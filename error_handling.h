#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

/**
 * @file error_handling.h
 * @brief Comprehensive error handling system for the space game
 * 
 * This module provides a unified error handling system with different severity levels,
 * error recovery mechanisms, and debugging support.
 */

// Error severity levels
typedef enum {
    ERROR_LEVEL_DEBUG,
    ERROR_LEVEL_INFO,
    ERROR_LEVEL_WARNING,
    ERROR_LEVEL_ERROR,
    ERROR_LEVEL_CRITICAL
} ErrorLevel;

// Error categories
typedef enum {
    ERROR_CATEGORY_GENERAL,
    ERROR_CATEGORY_GRAPHICS,
    ERROR_CATEGORY_AI,
    ERROR_CATEGORY_PHYSICS,
    ERROR_CATEGORY_NETWORK,
    ERROR_CATEGORY_MEMORY,
    ERROR_CATEGORY_FILE_IO
} ErrorCategory;

// Error codes
typedef enum {
    ERROR_SUCCESS = 0,
    ERROR_NULL_POINTER,
    ERROR_INVALID_PARAMETER,
    ERROR_OUT_OF_MEMORY,
    ERROR_FILE_NOT_FOUND,
    ERROR_INITIALIZATION_FAILED,
    ERROR_SDL_ERROR,
    ERROR_AI_MODEL_LOAD_FAILED,
    ERROR_ENTITY_NOT_FOUND,
    ERROR_UNIVERSE_FULL,
    ERROR_INVALID_STATE,
    ERROR_TIMEOUT,
    ERROR_UNKNOWN
} ErrorCode;

// Error information structure
struct ErrorInfo {
    ErrorCode code;
    ErrorLevel level;
    ErrorCategory category;
    char* message;
    char* file;
    int line;
    char* function;
    double timestamp;
};

// Error handling configuration
struct ErrorConfig {
    bool log_to_console;
    bool log_to_file;
    bool abort_on_critical;
    ErrorLevel min_log_level;
    char* log_file_path;
    FILE* log_file;
};

// Global error configuration
extern struct ErrorConfig g_error_config;


/**
 * @brief Initialize the error handling system
 * @param config Error handling configuration
 * @return true if successful, false otherwise
 */
bool error_system_init(struct ErrorConfig* config);

/**
 * @brief Shutdown the error handling system
 */
void error_system_shutdown(void);

/**
 * @brief Log an error with detailed information
 * @param code Error code
 * @param level Error severity level
 * @param category Error category
 * @param file Source file name
 * @param line Source line number
 * @param function Function name
 * @param format Printf-style format string
 * @param ... Format arguments
 */
void error_log_detailed(ErrorCode code, ErrorLevel level, ErrorCategory category,
                       const char* file, int line, const char* function,
                       const char* format, ...);

/**
 * @brief Get error message for an error code
 * @param code Error code
 * @return Error message string
 */
const char* error_get_message(ErrorCode code);

/**
 * @brief Check if an error is recoverable
 * @param code Error code
 * @return true if recoverable, false otherwise
 */
bool error_is_recoverable(ErrorCode code);

/**
 * @brief Get the last error that occurred
 * @return Pointer to ErrorInfo structure
 */
const struct ErrorInfo* error_get_last(void);

/**
 * @brief Clear the last error
 */
void error_clear_last(void);

// Convenience macros for error logging
#define LOG_DEBUG(category, format, ...) \
    error_log_detailed(ERROR_SUCCESS, ERROR_LEVEL_DEBUG, category, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define LOG_INFO(category, format, ...) \
    error_log_detailed(ERROR_SUCCESS, ERROR_LEVEL_INFO, category, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define LOG_WARNING(category, code, format, ...) \
    error_log_detailed(code, ERROR_LEVEL_WARNING, category, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define LOG_ERROR(category, code, format, ...) \
    error_log_detailed(code, ERROR_LEVEL_ERROR, category, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define LOG_CRITICAL(category, code, format, ...) \
    error_log_detailed(code, ERROR_LEVEL_CRITICAL, category, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

// Parameter validation macros
#define VALIDATE_PARAM(param, error_code) \
    do { \
        if (!(param)) { \
            LOG_ERROR(ERROR_CATEGORY_GENERAL, error_code, "Parameter validation failed: %s", #param); \
            return false; \
        } \
    } while(0)

#define VALIDATE_PARAM_RETURN_NULL(param, error_code) \
    do { \
        if (!(param)) { \
            LOG_ERROR(ERROR_CATEGORY_GENERAL, error_code, "Parameter validation failed: %s", #param); \
            return NULL; \
        } \
    }

#define VALIDATE_PARAM_RETURN_CODE(param, error_code) \
    do { \
        if (!(param)) { \
            LOG_ERROR(ERROR_CATEGORY_GENERAL, error_code, "Parameter validation failed: %s", #param); \
            return error_code; \
        } \
    } while(0)

// Memory allocation with error checking
#define SAFE_MALLOC(ptr, size, error_code) \
    do { \
        (ptr) = malloc(size); \
        if (!(ptr)) { \
            LOG_ERROR(ERROR_CATEGORY_MEMORY, error_code, "Memory allocation failed: %zu bytes", (size_t)(size)); \
            return false; \
        } \
    } while(0)

#define SAFE_MALLOC_RETURN_NULL(ptr, size, error_code) \
    do { \
        (ptr) = malloc(size); \
        if (!(ptr)) { \
            LOG_ERROR(ERROR_CATEGORY_MEMORY, error_code, "Memory allocation failed: %zu bytes", (size_t)(size)); \
            return NULL; \
        } \
    } while(0)

// SDL error checking
#define CHECK_SDL_ERROR(operation, error_msg) \
    do { \
        if ((operation) < 0) { \
            LOG_ERROR(ERROR_CATEGORY_GRAPHICS, ERROR_SDL_ERROR, "%s: %s", error_msg, SDL_GetError()); \
            return false; \
        } \
    } while(0)

// AI error checking
#define CHECK_AI_RESULT(result, error_msg) \
    do { \
        if (!(result)) { \
            LOG_ERROR(ERROR_CATEGORY_AI, ERROR_AI_MODEL_LOAD_FAILED, "%s", error_msg); \
            return false; \
        } \
    } while(0)

#endif // ERROR_HANDLING_H
