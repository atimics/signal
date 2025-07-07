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

// Get the controller configuration service
struct ControllerConfigService* game_input_get_controller_config_service(void);

// Controller detection and calibration functions
void game_input_check_new_controllers(void);
bool game_input_has_pending_calibration(void);
const char* game_input_get_pending_calibration_id(void);
void game_input_clear_pending_calibration(void);
void game_input_request_controller_calibration(const char* controller_id);

#endif // GAME_INPUT_H