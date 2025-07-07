/**
 * @file debug_logging.h
 * @brief Conditional debug logging macros to replace printf statements
 */

#ifndef DEBUG_LOGGING_H
#define DEBUG_LOGGING_H

#include <stdio.h>

// Debug logging categories
#ifdef DEBUG
  #define LOG_INPUT(fmt, ...)    printf("[INPUT] " fmt "\n", ##__VA_ARGS__)
  #define LOG_PHYSICS(fmt, ...)  printf("[PHYS] " fmt "\n", ##__VA_ARGS__)
  #define LOG_RENDER(fmt, ...)   printf("[RENDER] " fmt "\n", ##__VA_ARGS__)
  #define LOG_SYSTEM(fmt, ...)   printf("[SYSTEM] " fmt "\n", ##__VA_ARGS__)
  #define LOG_SCENE(fmt, ...)    printf("[SCENE] " fmt "\n", ##__VA_ARGS__)
  #define LOG_TODO(fmt, ...)     printf("[TODO] " fmt "\n", ##__VA_ARGS__)
#else
  #define LOG_INPUT(fmt, ...)
  #define LOG_PHYSICS(fmt, ...)
  #define LOG_RENDER(fmt, ...)
  #define LOG_SYSTEM(fmt, ...)
  #define LOG_SCENE(fmt, ...)
  #define LOG_TODO(fmt, ...)
#endif

// Always-on logging for important messages
#define LOG_INFO(fmt, ...)     printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)     printf("[WARN] " fmt "\n", ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)    printf("[ERROR] " fmt "\n", ##__VA_ARGS__)

// Success messages (keep emojis for user feedback)
#define LOG_SUCCESS(fmt, ...)  printf("✅ " fmt "\n", ##__VA_ARGS__)

#endif // DEBUG_LOGGING_H