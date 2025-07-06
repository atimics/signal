/**
 * @file input_compat.h
 * @brief Compatibility layer for legacy input system
 * 
 * This provides a bridge between the old input_mapping_* API
 * and the new InputService architecture during migration.
 */

#ifndef INPUT_COMPAT_H
#define INPUT_COMPAT_H

#include <stdbool.h>
#include <stdint.h>
#include "services/input_service.h"

// Initialize compatibility layer with the new service
void input_compat_init(InputService* service);
void input_compat_shutdown(void);

// Legacy API implementation using new service
bool input_mapping_just_pressed(uint32_t action);
bool input_mapping_pressed(uint32_t action);
bool input_mapping_just_released(uint32_t action);
float input_mapping_value(uint32_t action);

// Map legacy action IDs to new ones
InputActionID input_compat_map_legacy_action(uint32_t legacy_action);

// Process input frame (called from game loop)
void input_compat_process_frame(float delta_time);

#endif // INPUT_COMPAT_H