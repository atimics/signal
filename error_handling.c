#include "cgame.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

/**
 * @file error_handling.c
 * @brief Implementation of the error handling system
 */

// Global error configuration
struct ErrorConfig g_error_config = {
    .log_to_console = true,
    .log_to_file = false,
    .abort_on_critical = true,
    .min_log_level = ERROR_LEVEL_INFO,
    .log_file_path = NULL,
    .log_file = NULL
};

// Last error information
static struct ErrorInfo g_last_error = {0};

// Error level names for logging
static const char* error_level_names[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "CRITICAL"
};

// Error category names for logging
static const char* error_category_names[] = {
    "GENERAL",
    "GRAPHICS",
    "AI",
    "PHYSICS",
    "NETWORK",
    "MEMORY",
    "FILE_IO"
};

// Error code messages
static const char* error_code_messages[] = {
    [ERROR_SUCCESS] = "Success",
    [ERROR_NULL_POINTER] = "Null pointer error",
    [ERROR_INVALID_PARAMETER] = "Invalid parameter",
    [ERROR_OUT_OF_MEMORY] = "Out of memory",
    [ERROR_FILE_NOT_FOUND] = "File not found",
    [ERROR_INITIALIZATION_FAILED] = "Initialization failed",
    [ERROR_SDL_ERROR] = "SDL error",
    [ERROR_AI_MODEL_LOAD_FAILED] = "AI model load failed",
    [ERROR_ENTITY_NOT_FOUND] = "Entity not found",
    [ERROR_UNIVERSE_FULL] = "Universe at maximum capacity",
    [ERROR_INVALID_STATE] = "Invalid state",
    [ERROR_TIMEOUT] = "Operation timed out",
    [ERROR_UNKNOWN] = "Unknown error"
};

/**
 * @brief Get current timestamp in seconds since epoch
 * @return Timestamp as double
 */
static double get_timestamp(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (tv.tv_usec / 1000000.0);
}

bool error_system_init(struct ErrorConfig* config) {
    if (config) {
        g_error_config = *config;
    }
    
    // Open log file if configured
    if (g_error_config.log_to_file && g_error_config.log_file_path) {
        g_error_config.log_file = fopen(g_error_config.log_file_path, "a");
        if (!g_error_config.log_file) {
            fprintf(stderr, "ERROR: Failed to open log file: %s\n", g_error_config.log_file_path);
            return false;
        }
        
        // Write initialization message
        fprintf(g_error_config.log_file, "\n=== Error System Initialized at %.3f ===\n", get_timestamp());
        fflush(g_error_config.log_file);
    }
    
    // Clear last error
    memset(&g_last_error, 0, sizeof(struct ErrorInfo));
    
    printf("✅ Error handling system initialized\n");
    return true;
}

void error_system_shutdown(void) {
    if (g_error_config.log_file) {
        fprintf(g_error_config.log_file, "=== Error System Shutdown at %.3f ===\n\n", get_timestamp());
        fclose(g_error_config.log_file);
        g_error_config.log_file = NULL;
    }
    
    // Clean up last error message
    if (g_last_error.message) {
        free(g_last_error.message);
        g_last_error.message = NULL;
    }
    if (g_last_error.file) {
        free(g_last_error.file);
        g_last_error.file = NULL;
    }
    if (g_last_error.function) {
        free(g_last_error.function);
        g_last_error.function = NULL;
    }
    
    printf("🔄 Error handling system shutdown\n");
}

void error_log_detailed(ErrorCode code, ErrorLevel level, ErrorCategory category,
                       const char* file, int line, const char* function,
                       const char* format, ...) {
    
    // Check if we should log this level
    if (level < g_error_config.min_log_level) {
        return;
    }
    
    // Build message from format string
    char message_buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(message_buffer, sizeof(message_buffer), format, args);
    va_end(args);
    
    // Update last error
    if (g_last_error.message) free(g_last_error.message);
    if (g_last_error.file) free(g_last_error.file);
    if (g_last_error.function) free(g_last_error.function);
    
    g_last_error.code = code;
    g_last_error.level = level;
    g_last_error.category = category;
    g_last_error.message = strdup(message_buffer);
    g_last_error.file = strdup(file ? file : "unknown");
    g_last_error.line = line;
    g_last_error.function = strdup(function ? function : "unknown");
    g_last_error.timestamp = get_timestamp();
    
    // Format log entry
    char log_entry[2048];
    snprintf(log_entry, sizeof(log_entry),
        "[%.3f] %s/%s: %s (in %s:%d %s())",
        g_last_error.timestamp,
        error_level_names[level],
        error_category_names[category],
        message_buffer,
        file ? file : "unknown",
        line,
        function ? function : "unknown"
    );
    
    // Log to console
    if (g_error_config.log_to_console) {
        // Use different colors for different levels
        const char* color_code = "";
        const char* reset_code = "\033[0m";
        
        switch (level) {
            case ERROR_LEVEL_DEBUG:   color_code = "\033[36m"; break; // Cyan
            case ERROR_LEVEL_INFO:    color_code = "\033[32m"; break; // Green
            case ERROR_LEVEL_WARNING: color_code = "\033[33m"; break; // Yellow
            case ERROR_LEVEL_ERROR:   color_code = "\033[31m"; break; // Red
            case ERROR_LEVEL_CRITICAL: color_code = "\033[35m"; break; // Magenta
        }
        
        fprintf(stderr, "%s%s%s\n", color_code, log_entry, reset_code);
    }
    
    // Log to file
    if (g_error_config.log_to_file && g_error_config.log_file) {
        fprintf(g_error_config.log_file, "%s\n", log_entry);
        fflush(g_error_config.log_file);
    }
    
    // Handle critical errors
    if (level == ERROR_LEVEL_CRITICAL && g_error_config.abort_on_critical) {
        fprintf(stderr, "💥 CRITICAL ERROR: Aborting program\n");
        abort();
    }
}

const char* error_get_message(ErrorCode code) {
    if (code >= 0 && code < (sizeof(error_code_messages) / sizeof(error_code_messages[0]))) {
        return error_code_messages[code];
    }
    return error_code_messages[ERROR_UNKNOWN];
}

bool error_is_recoverable(ErrorCode code) {
    switch (code) {
        case ERROR_SUCCESS:
        case ERROR_ENTITY_NOT_FOUND:
        case ERROR_TIMEOUT:
        case ERROR_FILE_NOT_FOUND:
            return true;
            
        case ERROR_OUT_OF_MEMORY:
        case ERROR_INITIALIZATION_FAILED:
        case ERROR_AI_MODEL_LOAD_FAILED:
        case ERROR_SDL_ERROR:
            return false;
            
        default:
            return true; // Assume recoverable by default
    }
}

const struct ErrorInfo* error_get_last(void) {
    return &g_last_error;
}

void error_clear_last(void) {
    if (g_last_error.message) {
        free(g_last_error.message);
        g_last_error.message = NULL;
    }
    if (g_last_error.file) {
        free(g_last_error.file);
        g_last_error.file = NULL;
    }
    if (g_last_error.function) {
        free(g_last_error.function);
        g_last_error.function = NULL;
    }
    
    memset(&g_last_error, 0, sizeof(struct ErrorInfo));
}
