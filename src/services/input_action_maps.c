#include "input_action_maps.h"
#include "../utils/json_parser.h"
#include "input_constants.h"
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
    
    printf("INFO: Loading action maps from %s\n", file_path);
    
    // Read the file
    FILE* file = fopen(file_path, "r");
    if (!file) {
        printf("INFO: Could not open action maps file: %s\n", file_path);
        return false;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Read entire file
    char* json_string = malloc(file_size + 1);
    if (!json_string) {
        fclose(file);
        return false;
    }
    
    size_t read_size = fread(json_string, 1, file_size, file);
    json_string[read_size] = '\0';
    fclose(file);
    
    // Parse JSON
    JSONValue* root = json_parse(json_string);
    free(json_string);
    
    if (!root || root->type != JSON_OBJECT) {
        printf("ERROR: Invalid JSON format in action maps file\n");
        if (root) json_free(root);
        return false;
    }
    
    // Get contexts object
    JSONValue* contexts = json_object_get(root, "contexts");
    if (!contexts || contexts->type != JSON_OBJECT) {
        printf("ERROR: Missing 'contexts' object in action maps\n");
        json_free(root);
        return false;
    }
    
    // Parse each context (e.g., "gameplay", "menu")
    for (size_t ctx_idx = 0; ctx_idx < contexts->data.object.count; ctx_idx++) {
        const char* context_name = contexts->data.object.keys[ctx_idx];
        JSONValue* context_obj = contexts->data.object.values[ctx_idx];
        
        if (context_obj->type != JSON_OBJECT) continue;
        
        // Create action map for this context
        if (!manager->add_map(manager, context_name)) {
            printf("WARNING: Could not add map '%s'\n", context_name);
            continue;
        }
        
        // Get actions object
        JSONValue* actions = json_object_get(context_obj, "actions");
        if (!actions || actions->type != JSON_OBJECT) continue;
        
        // Parse each action
        for (size_t act_idx = 0; act_idx < actions->data.object.count; act_idx++) {
            const char* action_name = actions->data.object.keys[act_idx];
            JSONValue* bindings_array = actions->data.object.values[act_idx];
            
            if (bindings_array->type != JSON_ARRAY) continue;
            
            // Map action name to InputActionID
            InputActionID action_id = INPUT_ACTION_NONE;
            
            // Movement actions
            if (strcmp(action_name, "thrust_forward") == 0) action_id = INPUT_ACTION_THRUST_FORWARD;
            else if (strcmp(action_name, "thrust_back") == 0) action_id = INPUT_ACTION_THRUST_BACK;
            else if (strcmp(action_name, "thrust_left") == 0) action_id = INPUT_ACTION_MOVE_LEFT;
            else if (strcmp(action_name, "thrust_right") == 0) action_id = INPUT_ACTION_MOVE_RIGHT;
            else if (strcmp(action_name, "vertical_up") == 0) action_id = INPUT_ACTION_VERTICAL_UP;
            else if (strcmp(action_name, "vertical_down") == 0) action_id = INPUT_ACTION_VERTICAL_DOWN;
            
            // Rotation actions
            else if (strcmp(action_name, "pitch_up") == 0) action_id = INPUT_ACTION_PITCH_UP;
            else if (strcmp(action_name, "pitch_down") == 0) action_id = INPUT_ACTION_PITCH_DOWN;
            else if (strcmp(action_name, "yaw_left") == 0) action_id = INPUT_ACTION_YAW_LEFT;
            else if (strcmp(action_name, "yaw_right") == 0) action_id = INPUT_ACTION_YAW_RIGHT;
            else if (strcmp(action_name, "roll_left") == 0) action_id = INPUT_ACTION_ROLL_LEFT;
            else if (strcmp(action_name, "roll_right") == 0) action_id = INPUT_ACTION_ROLL_RIGHT;
            
            // Control actions
            else if (strcmp(action_name, "boost") == 0) action_id = INPUT_ACTION_BOOST;
            else if (strcmp(action_name, "brake") == 0) action_id = INPUT_ACTION_BRAKE;
            
            // UI actions
            else if (strcmp(action_name, "ui_up") == 0) action_id = INPUT_ACTION_UI_UP;
            else if (strcmp(action_name, "ui_down") == 0) action_id = INPUT_ACTION_UI_DOWN;
            else if (strcmp(action_name, "ui_left") == 0) action_id = INPUT_ACTION_UI_LEFT;
            else if (strcmp(action_name, "ui_right") == 0) action_id = INPUT_ACTION_UI_RIGHT;
            else if (strcmp(action_name, "ui_confirm") == 0) action_id = INPUT_ACTION_UI_CONFIRM;
            else if (strcmp(action_name, "ui_cancel") == 0) action_id = INPUT_ACTION_UI_CANCEL;
            else if (strcmp(action_name, "ui_menu") == 0) action_id = INPUT_ACTION_UI_MENU;
            
            if (action_id == INPUT_ACTION_NONE) {
                printf("WARNING: Unknown action '%s'\n", action_name);
                continue;
            }
            
            // Parse each binding for this action
            for (size_t bind_idx = 0; bind_idx < bindings_array->data.array.count; bind_idx++) {
                JSONValue* binding = json_array_get(bindings_array, bind_idx);
                if (!binding || binding->type != JSON_OBJECT) continue;
                
                // Get device type
                JSONValue* device_val = json_object_get(binding, "device");
                if (!device_val || device_val->type != JSON_STRING) continue;
                
                const char* device_str = json_get_string(device_val);
                InputDeviceType device_type = INPUT_DEVICE_KEYBOARD;
                
                if (strcmp(device_str, "keyboard") == 0) {
                    device_type = INPUT_DEVICE_KEYBOARD;
                    
                    // Get key
                    JSONValue* key_val = json_object_get(binding, "key");
                    if (!key_val) continue;
                    
                    int32_t key_code = 0;
                    if (key_val->type == JSON_STRING) {
                        const char* key_str = json_get_string(key_val);
                        if (strlen(key_str) == 1) {
                            key_code = key_str[0]; // Single character
                        } else {
                            // Named keys
                            if (strcmp(key_str, "space") == 0) key_code = ' ';
                            else if (strcmp(key_str, "enter") == 0) key_code = SAPP_KEYCODE_ENTER;
                            else if (strcmp(key_str, "escape") == 0) key_code = SAPP_KEYCODE_ESCAPE;
                            else if (strcmp(key_str, "up") == 0) key_code = SAPP_KEYCODE_UP;
                            else if (strcmp(key_str, "down") == 0) key_code = SAPP_KEYCODE_DOWN;
                            else if (strcmp(key_str, "left") == 0) key_code = SAPP_KEYCODE_LEFT;
                            else if (strcmp(key_str, "right") == 0) key_code = SAPP_KEYCODE_RIGHT;
                            else if (strcmp(key_str, "ctrl") == 0) key_code = SAPP_KEYCODE_LEFT_CONTROL;
                            else if (strcmp(key_str, "shift") == 0) key_code = SAPP_KEYCODE_LEFT_SHIFT;
                            else if (strcmp(key_str, "alt") == 0) key_code = SAPP_KEYCODE_LEFT_ALT;
                        }
                    } else if (key_val->type == JSON_NUMBER) {
                        key_code = (int32_t)json_get_number(key_val);
                    }
                    
                    if (key_code != 0) {
                        manager->add_binding(manager, context_name, action_id, device_type, key_code, 0);
                    }
                    
                } else if (strcmp(device_str, "gamepad") == 0) {
                    device_type = INPUT_DEVICE_GAMEPAD;
                    
                    // Get gamepad ID (default to 0)
                    JSONValue* id_val = json_object_get(binding, "id");
                    int32_t gamepad_id = id_val ? (int32_t)json_get_number(id_val) : 0;
                    
                    // Check if it's an axis or button
                    JSONValue* axis_val = json_object_get(binding, "axis");
                    JSONValue* button_val = json_object_get(binding, "button");
                    
                    if (axis_val && axis_val->type == JSON_NUMBER) {
                        int32_t axis = (int32_t)json_get_number(axis_val);
                        // For axes, we encode the axis number in the key_or_button field
                        // with a high bit set to indicate it's an axis
                        manager->add_binding(manager, context_name, action_id, device_type, 
                                           axis | 0x80000000, gamepad_id);
                    } else if (button_val && button_val->type == JSON_NUMBER) {
                        int32_t button = (int32_t)json_get_number(button_val);
                        manager->add_binding(manager, context_name, action_id, device_type, 
                                           button, gamepad_id);
                    }
                }
            }
        }
    }
    
    json_free(root);
    
    printf("INFO: Successfully loaded action maps from %s\n", file_path);
    printf("INFO: Loaded %d action maps\n", manager->collection->map_count);
    
    return manager->collection->map_count > 0;
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
    
    printf("INFO: Applying action maps to input service\n");
    
    // Apply all loaded action maps to the service
    for (int i = 0; i < manager->collection->map_count; i++) {
        ActionMap* map = &manager->collection->maps[i];
        if (!map->is_active) continue;
        
        // Determine which context this map belongs to
        InputContextID context = INPUT_CONTEXT_MENU;
        if (strcmp(map->name, "gameplay") == 0 || strcmp(map->name, "flight") == 0) {
            context = INPUT_CONTEXT_GAMEPLAY;
        }
        
        printf("  Applying %d bindings from map '%s' to context %d\n", 
               map->binding_count, map->name, context);
        
        // Apply each binding
        for (int b = 0; b < map->binding_count; b++) {
            InputBinding binding = {0};
            binding.device = map->bindings[b].device_type;
            binding.scale = 1.0f;
            
            switch (binding.device) {
                case INPUT_DEVICE_KEYBOARD:
                    binding.binding.keyboard.key = map->bindings[b].key_or_button;
                    break;
                case INPUT_DEVICE_GAMEPAD:
                    binding.binding.gamepad.gamepad_id = map->bindings[b].device_id;
                    // Check if this is an axis (high bit set) or button
                    if (map->bindings[b].key_or_button & 0x80000000) {
                        // It's an axis
                        binding.binding.gamepad.axis = map->bindings[b].key_or_button & 0x7FFFFFFF;
                        binding.binding.gamepad.is_axis = true;
                        // TODO: Store scale values in ActionMapBinding and use them here
                        binding.scale = 1.0f; // Default scale for now
                    } else {
                        // It's a button
                        binding.binding.gamepad.button = map->bindings[b].key_or_button;
                        binding.binding.gamepad.is_axis = false;
                    }
                    break;
                default:
                    continue;
            }
            
            service->bind_action(service, map->bindings[b].action, context, &binding);
        }
    }
    
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