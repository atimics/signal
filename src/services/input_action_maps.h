#ifndef INPUT_ACTION_MAPS_H
#define INPUT_ACTION_MAPS_H

#include <stdbool.h>
#include <stdint.h>

// Include necessary types
#include "../hal/input_hal.h"
#include "input_service.h"

// Maximum limits for action maps
#define MAX_ACTION_MAPS 16
#define MAX_BINDINGS_PER_MAP 64
#define MAX_MAP_NAME_LENGTH 64

// Input binding structure for action maps
typedef struct {
    InputActionID action;        // The action this binding triggers
    InputDeviceType device_type; // Keyboard, mouse, gamepad
    int32_t key_or_button;       // Specific key code or button index
    int32_t device_id;           // Gamepad ID (0 for keyboard/mouse)
    bool is_active;              // Whether this binding is enabled
} ActionMapBinding;

// Action map structure (e.g., "flight", "menu")
typedef struct {
    char name[MAX_MAP_NAME_LENGTH];
    ActionMapBinding bindings[MAX_BINDINGS_PER_MAP];
    int binding_count;
    bool is_active;
} ActionMap;

// Collection of action maps
typedef struct {
    ActionMap maps[MAX_ACTION_MAPS];
    int map_count;
} ActionMapCollection;

// Action map manager with function pointers for operations
typedef struct ActionMapManager {
    ActionMapCollection* collection;
    bool (*load_from_file)(struct ActionMapManager* manager, const char* file_path);
    bool (*save_to_file)(struct ActionMapManager* manager, const char* file_path);
    bool (*add_map)(struct ActionMapManager* manager, const char* name);
    bool (*add_binding)(struct ActionMapManager* manager, const char* map_name, InputActionID action, InputDeviceType device_type, int32_t key_or_button, int32_t device_id);
    bool (*apply_to_service)(struct ActionMapManager* manager, InputService* service);
} ActionMapManager;

// Core functions
ActionMapCollection* action_map_collection_create(void);
void action_map_collection_destroy(ActionMapCollection* collection);

ActionMapManager* action_map_manager_create(void);
void action_map_manager_destroy(ActionMapManager* manager);

// Legacy compatibility function
bool action_maps_load_and_apply(struct InputService* service, const char* file_path);

#endif // INPUT_ACTION_MAPS_H