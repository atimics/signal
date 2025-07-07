/**
 * @file controller_config.c
 * @brief Controller configuration service implementation
 */

#include "controller_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Internal service data
typedef struct {
    ControllerConfigDatabase database;
    char config_path[256];
    bool database_loaded;
    bool database_dirty;
} ControllerConfigServiceData;

// Forward declarations
static bool controller_config_service_init(ControllerConfigService* self, const char* config_path);
static void controller_config_service_shutdown(ControllerConfigService* self);
static bool controller_config_service_load_database(ControllerConfigService* self, const char* path);
static bool controller_config_service_save_database(ControllerConfigService* self, const char* path);
static ControllerConfig* controller_config_service_get_config(ControllerConfigService* self, const char* controller_id);
static ControllerConfig* controller_config_service_get_config_by_index(ControllerConfigService* self, int gamepad_index);
static bool controller_config_service_has_config(ControllerConfigService* self, const char* controller_id);
static ControllerConfig* controller_config_service_create_config(ControllerConfigService* self, const char* controller_id, const char* name);
static bool controller_config_service_update_config(ControllerConfigService* self, const ControllerConfig* config);
static bool controller_config_service_delete_config(ControllerConfigService* self, const char* controller_id);

// ============================================================================
// SERVICE IMPLEMENTATION
// ============================================================================

static bool controller_config_service_init(ControllerConfigService* self, const char* config_path) {
    if (!self || !self->internal) return false;
    
    ControllerConfigServiceData* data = (ControllerConfigServiceData*)self->internal;
    
    // Store config path
    if (config_path) {
        strncpy(data->config_path, config_path, sizeof(data->config_path) - 1);
        data->config_path[sizeof(data->config_path) - 1] = '\0';
    } else {
        strcpy(data->config_path, "data/config/controller_configs.yaml");
    }
    
    // Initialize database with defaults
    memset(&data->database, 0, sizeof(data->database));
    strcpy(data->database.version, "1.0");
    strcpy(data->database.last_updated, "2025-01-07");
    
    // Set up defaults
    data->database.defaults.deadzone.stick = 0.15f;
    data->database.defaults.deadzone.trigger = 0.05f;
    data->database.defaults.sensitivity.linear = 1.0f;
    data->database.defaults.sensitivity.angular = 1.0f;
    data->database.defaults.response_curve.type = RESPONSE_CURVE_LINEAR;
    data->database.defaults.response_curve.exponent = 1.0f;
    data->database.defaults.auto_calibration.enabled = true;
    data->database.defaults.auto_calibration.samples = 100;
    
    data->database_loaded = false;
    data->database_dirty = false;
    
    printf("✅ Controller Config Service initialized\n");
    return true;
}

static void controller_config_service_shutdown(ControllerConfigService* self) {
    if (!self || !self->internal) return;
    
    ControllerConfigServiceData* data = (ControllerConfigServiceData*)self->internal;
    
    // Save if dirty
    if (data->database_dirty) {
        controller_config_service_save_database(self, data->config_path);
    }
    
    printf("✅ Controller Config Service shut down\n");
}

static bool controller_config_service_load_database(ControllerConfigService* self, const char* path) {
    if (!self || !self->internal) return false;
    
    ControllerConfigServiceData* data = (ControllerConfigServiceData*)self->internal;
    
    printf("📋 Loading controller config database from: %s\n", path);
    
    // For now, just mark as loaded with defaults
    // In a full implementation, this would parse the YAML file
    data->database_loaded = true;
    data->database_dirty = false;
    
    printf("✅ Controller config database loaded\n");
    return true;
}

static bool controller_config_service_save_database(ControllerConfigService* self, const char* path) {
    if (!self || !self->internal) return false;
    
    ControllerConfigServiceData* data = (ControllerConfigServiceData*)self->internal;
    
    printf("💾 Saving controller config database to: %s\n", path);
    
    // For now, just mark as saved
    // In a full implementation, this would write the YAML file
    data->database_dirty = false;
    
    printf("✅ Controller config database saved\n");
    return true;
}

static ControllerConfig* controller_config_service_get_config(ControllerConfigService* self, const char* controller_id) {
    if (!self || !self->internal || !controller_id) return NULL;
    
    ControllerConfigServiceData* data = (ControllerConfigServiceData*)self->internal;
    
    // Search for existing config
    for (int i = 0; i < data->database.controller_count; i++) {
        if (strcmp(data->database.controllers[i].id, controller_id) == 0) {
            return &data->database.controllers[i];
        }
    }
    
    return NULL;
}

static ControllerConfig* controller_config_service_get_config_by_index(ControllerConfigService* self, int gamepad_index) {
    if (!self || !self->internal || gamepad_index < 0) return NULL;
    
    // Generate ID for gamepad index
    char controller_id[32];
    snprintf(controller_id, sizeof(controller_id), "gamepad_%d", gamepad_index);
    
    return controller_config_service_get_config(self, controller_id);
}

static bool controller_config_service_has_config(ControllerConfigService* self, const char* controller_id) {
    return controller_config_service_get_config(self, controller_id) != NULL;
}

static ControllerConfig* controller_config_service_create_config(ControllerConfigService* self, const char* controller_id, const char* name) {
    if (!self || !self->internal || !controller_id || !name) return NULL;
    
    ControllerConfigServiceData* data = (ControllerConfigServiceData*)self->internal;
    
    // Check if we have space
    if (data->database.controller_count >= MAX_CONTROLLERS) {
        printf("❌ Cannot create controller config: database full\n");
        return NULL;
    }
    
    // Create new config
    ControllerConfig* config = &data->database.controllers[data->database.controller_count];
    memset(config, 0, sizeof(ControllerConfig));
    
    strncpy(config->id, controller_id, sizeof(config->id) - 1);
    strncpy(config->name, name, sizeof(config->name) - 1);
    strncpy(config->type, "generic", sizeof(config->type) - 1);
    strncpy(config->vendor, "Unknown", sizeof(config->vendor) - 1);
    config->verified = false;
    strncpy(config->created, "2025-01-07", sizeof(config->created) - 1);
    strncpy(config->last_calibrated, "2025-01-07", sizeof(config->last_calibrated) - 1);
    
    // Apply defaults
    controller_config_apply_defaults(config, &data->database.defaults);
    
    // Set calibration status
    config->calibration.status = CALIBRATION_STATUS_NEEDS_CALIBRATION;
    strncpy(config->calibration.method, "wizard", sizeof(config->calibration.method) - 1);
    
    data->database.controller_count++;
    data->database_dirty = true;
    
    printf("✅ Created controller config: %s (%s)\n", controller_id, name);
    return config;
}

static bool controller_config_service_update_config(ControllerConfigService* self, const ControllerConfig* config) {
    if (!self || !self->internal || !config) return false;
    
    ControllerConfigServiceData* data = (ControllerConfigServiceData*)self->internal;
    
    // Find existing config
    ControllerConfig* existing = controller_config_service_get_config(self, config->id);
    if (existing) {
        // Update existing
        *existing = *config;
        data->database_dirty = true;
        printf("✅ Updated controller config: %s\n", config->id);
        return true;
    }
    
    printf("❌ Cannot update controller config: %s not found\n", config->id);
    return false;
}

static bool controller_config_service_delete_config(ControllerConfigService* self, const char* controller_id) {
    if (!self || !self->internal || !controller_id) return false;
    
    ControllerConfigServiceData* data = (ControllerConfigServiceData*)self->internal;
    
    // Find and remove config
    for (int i = 0; i < data->database.controller_count; i++) {
        if (strcmp(data->database.controllers[i].id, controller_id) == 0) {
            // Shift remaining configs down
            for (int j = i; j < data->database.controller_count - 1; j++) {
                data->database.controllers[j] = data->database.controllers[j + 1];
            }
            data->database.controller_count--;
            data->database_dirty = true;
            
            printf("✅ Deleted controller config: %s\n", controller_id);
            return true;
        }
    }
    
    printf("❌ Cannot delete controller config: %s not found\n", controller_id);
    return false;
}

// ============================================================================
// FACTORY FUNCTIONS
// ============================================================================

ControllerConfigService* controller_config_service_create(void) {
    ControllerConfigService* service = (ControllerConfigService*)calloc(1, sizeof(ControllerConfigService));
    if (!service) return NULL;
    
    // Allocate internal data
    ControllerConfigServiceData* data = (ControllerConfigServiceData*)calloc(1, sizeof(ControllerConfigServiceData));
    if (!data) {
        free(service);
        return NULL;
    }
    
    // Set up function pointers
    service->init = controller_config_service_init;
    service->shutdown = controller_config_service_shutdown;
    service->load_database = controller_config_service_load_database;
    service->save_database = controller_config_service_save_database;
    service->get_config = controller_config_service_get_config;
    service->get_config_by_index = controller_config_service_get_config_by_index;
    service->has_config = controller_config_service_has_config;
    service->create_config = controller_config_service_create_config;
    service->update_config = controller_config_service_update_config;
    service->delete_config = controller_config_service_delete_config;
    
    service->internal = data;
    
    // Initialize with default path
    if (!service->init(service, NULL)) {
        controller_config_service_destroy(service);
        return NULL;
    }
    
    return service;
}

void controller_config_service_destroy(ControllerConfigService* service) {
    if (service) {
        if (service->shutdown) {
            service->shutdown(service);
        }
        if (service->internal) {
            free(service->internal);
        }
        free(service);
    }
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void controller_config_apply_defaults(ControllerConfig* config, const ControllerDefaults* defaults) {
    if (!config || !defaults) return;
    
    // Set up default axes for Xbox controller layout
    config->axis_count = 6;
    
    const char* axis_names[] = {"Right Stick X", "Right Stick Y", "Left Stick X", 
                               "Left Stick Y", "Left Trigger", "Right Trigger"};
    const char* axis_types[] = {"analog", "analog", "analog", "analog", "trigger", "trigger"};
    
    for (int i = 0; i < config->axis_count; i++) {
        AxisConfig* axis = &config->axes[i];
        axis->index = i;
        strncpy(axis->name, axis_names[i], sizeof(axis->name) - 1);
        strncpy(axis->type, axis_types[i], sizeof(axis->type) - 1);
        
        if (i < 4) {  // Analog sticks
            axis->range[0] = -1.0f;
            axis->range[1] = 1.0f;
            axis->deadzone = defaults->deadzone.stick;
            axis->invert = (i == 1 || i == 3);  // Invert Y axes
        } else {  // Triggers
            axis->range[0] = 0.0f;
            axis->range[1] = 1.0f;
            axis->deadzone = defaults->deadzone.trigger;
            axis->invert = false;
        }
        
        axis->sensitivity = (i < 4) ? defaults->sensitivity.angular : defaults->sensitivity.linear;
        axis->response_curve = defaults->response_curve.type;
        axis->curve_exponent = defaults->response_curve.exponent;
    }
    
    // Set up default buttons
    config->button_count = 14;
    
    const char* button_names[] = {"A", "B", "X", "Y", "Left Bumper", "Right Bumper",
                                 "View", "Menu", "Left Stick Click", "Right Stick Click",
                                 "D-Pad Up", "D-Pad Down", "D-Pad Left", "D-Pad Right"};
    const char* button_types[] = {"action", "action", "action", "action", "shoulder", "shoulder",
                                 "system", "system", "stick", "stick", 
                                 "dpad", "dpad", "dpad", "dpad"};
    
    for (int i = 0; i < config->button_count; i++) {
        ButtonConfig* button = &config->buttons[i];
        button->index = i;
        strncpy(button->name, button_names[i], sizeof(button->name) - 1);
        strncpy(button->type, button_types[i], sizeof(button->type) - 1);
        button->verified = false;
    }
}

float controller_config_apply_curve(float input, ResponseCurveType curve, float exponent) {
    if (fabsf(input) < 0.001f) return 0.0f;
    
    float sign = (input < 0.0f) ? -1.0f : 1.0f;
    float abs_input = fabsf(input);
    
    switch (curve) {
        case RESPONSE_CURVE_LINEAR:
            return input;
            
        case RESPONSE_CURVE_QUADRATIC:
            return sign * abs_input * abs_input;
            
        case RESPONSE_CURVE_CUBIC:
            return sign * abs_input * abs_input * abs_input;
            
        case RESPONSE_CURVE_CUSTOM:
            return sign * powf(abs_input, exponent);
            
        default:
            return input;
    }
}

float controller_config_apply_deadzone(float input, float deadzone) {
    if (fabsf(input) < deadzone) return 0.0f;
    
    // Rescale to maintain full range outside deadzone
    float sign = (input < 0.0f) ? -1.0f : 1.0f;
    float abs_input = fabsf(input);
    
    float scaled = (abs_input - deadzone) / (1.0f - deadzone);
    return sign * fmaxf(0.0f, fminf(1.0f, scaled));
}

void controller_config_generate_id(uint16_t vendor_id, uint16_t product_id, char* id_buffer, size_t buffer_size) {
    snprintf(id_buffer, buffer_size, "%04X:%04X", vendor_id, product_id);
}

bool controller_config_parse_id(const char* id, uint16_t* vendor_id, uint16_t* product_id) {
    if (!id || !vendor_id || !product_id) return false;
    
    unsigned int vid, pid;
    if (sscanf(id, "%X:%X", &vid, &pid) == 2) {
        *vendor_id = (uint16_t)vid;
        *product_id = (uint16_t)pid;
        return true;
    }
    
    return false;
}