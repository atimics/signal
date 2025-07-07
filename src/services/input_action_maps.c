#include "input_action_maps.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for manager function implementations
static bool manager_load_from_file(ActionMapManager* manager, const char* file_path);
static bool manager_save_to_file(ActionMapManager* manager, const char* file_path);
static bool manager_add_map(ActionMapManager* manager, const char* name);
static bool manager_add_binding(ActionMapManager* manager, const char* map_name, InputActionID action, InputDeviceType device_type, int32_t key_or_button, int32_t device_id);
static bool manager_apply_to_service(ActionMapManager* manager, InputService* service);

ActionMapCollection* action_map_collection_create(void) {
    ActionMapCollection* collection = malloc(sizeof(ActionMapCollection));
    if (!collection) return NULL;
    
    memset(collection, 0, sizeof(ActionMapCollection));
    collection->map_count = 0;
    
    return collection;
}

void action_map_collection_destroy(ActionMapCollection* collection) {
    if (collection) {
        free(collection);
    }
}

ActionMapManager* action_map_manager_create(void) {
    ActionMapManager* manager = malloc(sizeof(ActionMapManager));
    if (!manager) return NULL;
    
    manager->collection = action_map_collection_create();
    if (!manager->collection) {
        free(manager);
        return NULL;
    }
    
    // Set up function pointers
    manager->load_from_file = manager_load_from_file;
    manager->save_to_file = manager_save_to_file;
    manager->add_map = manager_add_map;
    manager->add_binding = manager_add_binding;
    manager->apply_to_service = manager_apply_to_service;
    
    return manager;
}

void action_map_manager_destroy(ActionMapManager* manager) {
    if (manager) {
        action_map_collection_destroy(manager->collection);
        free(manager);
    }
}

static bool manager_load_from_file(ActionMapManager* manager, const char* file_path) {
    if (!manager || !file_path) return false;
    
    // Stub implementation - in a real system this would parse JSON
    printf("INFO: Loading action maps from %s (stub implementation)\n", file_path);
    
    // Add a default flight map for testing
    manager_add_map(manager, "flight");
    manager_add_binding(manager, "flight", INPUT_ACTION_THRUST_FORWARD, INPUT_DEVICE_KEYBOARD, 87, 0); // W key
    manager_add_binding(manager, "flight", INPUT_ACTION_THRUST_BACK, INPUT_DEVICE_KEYBOARD, 83, 0);    // S key
    
    return true;
}

static bool manager_save_to_file(ActionMapManager* manager, const char* file_path) {
    if (!manager || !file_path) return false;
    
    printf("INFO: Saving action maps to %s (stub implementation)\n", file_path);
    return true;
}

static bool manager_add_map(ActionMapManager* manager, const char* name) {
    if (!manager || !name || manager->collection->map_count >= MAX_ACTION_MAPS) {
        return false;
    }
    
    ActionMap* map = &manager->collection->maps[manager->collection->map_count];
    strncpy(map->name, name, MAX_MAP_NAME_LENGTH - 1);
    map->name[MAX_MAP_NAME_LENGTH - 1] = '\0';
    map->binding_count = 0;
    map->is_active = true;
    
    manager->collection->map_count++;
    return true;
}

static bool manager_add_binding(ActionMapManager* manager, const char* map_name, InputActionID action, InputDeviceType device_type, int32_t key_or_button, int32_t device_id) {
    if (!manager || !map_name) return false;
    
    // Find the map
    ActionMap* target_map = NULL;
    for (int i = 0; i < manager->collection->map_count; i++) {
        if (strcmp(manager->collection->maps[i].name, map_name) == 0) {
            target_map = &manager->collection->maps[i];
            break;
        }
    }
    
    if (!target_map || target_map->binding_count >= MAX_BINDINGS_PER_MAP) {
        return false;
    }
    
    ActionMapBinding* binding = &target_map->bindings[target_map->binding_count];
    binding->action = action;
    binding->device_type = device_type;
    binding->key_or_button = key_or_button;
    binding->device_id = device_id;
    binding->is_active = true;
    
    target_map->binding_count++;
    return true;
}

static bool manager_apply_to_service(ActionMapManager* manager, InputService* service) {
    if (!manager || !service) return false;
    
    // Stub implementation - would apply bindings to the service
    printf("INFO: Applying action maps to input service (stub implementation)\n");
    return true;
}

bool action_maps_load_and_apply(InputService* service, const char* file_path) {
    if (!service) return false;
    
    ActionMapManager* manager = action_map_manager_create();
    if (!manager) return false;
    
    bool success = false;
    if (manager->load_from_file(manager, file_path)) {
        success = manager->apply_to_service(manager, service);
    }
    
    // Fallback to default bindings if loading fails
    if (!success) {
        printf("INFO: action_maps_load_and_apply() falling back to default bindings\n");
        input_service_setup_default_bindings(service);
        success = true;
    }
    
    action_map_manager_destroy(manager);
    return success;
}