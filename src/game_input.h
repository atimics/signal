/**
 * @file game_input.h
 * @brief Game-level input management system
 * 
 * Owns the InputService and HAL instances, provides initialization
 * and frame processing for the entire input system.
 */

#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include <stdbool.h>

// Initialize the game input system
bool game_input_init(void);

// Shutdown the game input system
void game_input_shutdown(void);

// Process input for the current frame
void game_input_process_frame(float delta_time);

// Check if the new input system is enabled (feature flag)
bool game_input_is_new_system_enabled(void);

// Get the global input service (for direct access if needed)
struct InputService* game_input_get_service(void);

#endif // GAME_INPUT_H