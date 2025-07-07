/**
 * @file controller_config.h
 * @brief Controller configuration and calibration system
 * 
 * Provides data-driven controller configuration with YAML storage,
 * automatic calibration wizards, and per-controller customization.
 */

#ifndef CONTROLLER_CONFIG_H
#define CONTROLLER_CONFIG_H

#include "../core.h"
#include "input_service.h"
#include <stdbool.h>
#include <stdint.h>

// Maximum limits
#define MAX_CONTROLLER_AXES 16
#define MAX_CONTROLLER_BUTTONS 32
#define MAX_CONTROLLER_NAME 64
#define MAX_CONTROLLER_ID 32
#define MAX_CONTROLLERS 16

// Response curve types
typedef enum {
    RESPONSE_CURVE_LINEAR = 0,
    RESPONSE_CURVE_QUADRATIC,
    RESPONSE_CURVE_CUBIC,
    RESPONSE_CURVE_CUSTOM
} ResponseCurveType;

// Calibration status
typedef enum {
    CALIBRATION_STATUS_NONE = 0,
    CALIBRATION_STATUS_NEEDS_CALIBRATION,
    CALIBRATION_STATUS_IN_PROGRESS,
    CALIBRATION_STATUS_COMPLETED,
    CALIBRATION_STATUS_VERIFIED
} CalibrationStatus;

// Axis configuration
typedef struct {
    int index;
    char name[32];
    char type[16];               // "analog", "trigger", "dpad"
    float range[2];              // [min, max]
    float deadzone;
    float sensitivity;
    bool invert;
    ResponseCurveType response_curve;
    float curve_exponent;
    
    // Calibration data
    float measured_range[2];     // Actual measured range
    float center_drift;          // Center position offset
    int calibration_samples;
} AxisConfig;

// Button configuration
typedef struct {
    int index;
    char name[32];
    char type[16];               // "action", "shoulder", "system", "stick", "dpad"
    bool verified;               // Has been tested/calibrated
} ButtonConfig;

// Flight control mapping
typedef struct {
    struct {
        char type[8];            // "axis" or "button"
        int index;
        float scale;
    } thrust_forward, thrust_back;
    
    struct {
        char type[8];
        int index;
        float scale;
    } pitch_up, pitch_down, yaw_left, yaw_right;
    
    struct {
        char type[8];
        int index;
        float scale;
    } roll_left, roll_right, vertical_up, vertical_down;
    
    struct {
        char type[8];
        int index;
    } boost, brake;
} FlightMapping;

// Calibration data
typedef struct {
    CalibrationStatus status;
    char method[16];             // "manual", "wizard", "auto"
    char date[32];
    int total_samples;
    bool auto_calibration_enabled;
    float auto_deadzone_threshold;
} CalibrationData;

// Complete controller configuration
typedef struct {
    char id[MAX_CONTROLLER_ID];          // VID:PID format
    char name[MAX_CONTROLLER_NAME];
    char type[32];
    char vendor[32];
    bool verified;
    char created[32];
    char last_calibrated[32];
    
    // Hardware layout
    int axis_count;
    AxisConfig axes[MAX_CONTROLLER_AXES];
    
    int button_count;
    ButtonConfig buttons[MAX_CONTROLLER_BUTTONS];
    
    // Control mappings
    FlightMapping flight_mapping;
    
    // Calibration info
    CalibrationData calibration;
    
} ControllerConfig;

// Global controller defaults
typedef struct {
    struct {
        float stick;
        float trigger;
    } deadzone;
    
    struct {
        float linear;
        float angular;
    } sensitivity;
    
    struct {
        ResponseCurveType type;
        float exponent;
    } response_curve;
    
    struct {
        bool enabled;
        int samples;
    } auto_calibration;
} ControllerDefaults;

// Controller configuration database
typedef struct {
    char version[16];
    char last_updated[32];
    
    ControllerDefaults defaults;
    
    int controller_count;
    ControllerConfig controllers[MAX_CONTROLLERS];
    
    ControllerConfig generic_template;
} ControllerConfigDatabase;

// Controller configuration service
typedef struct ControllerConfigService ControllerConfigService;

struct ControllerConfigService {
    // Internal data
    void* internal;
    
    // Core functions
    bool (*init)(ControllerConfigService* self, const char* config_path);
    void (*shutdown)(ControllerConfigService* self);
    
    // Configuration management
    bool (*load_database)(ControllerConfigService* self, const char* path);
    bool (*save_database)(ControllerConfigService* self, const char* path);
    
    // Controller lookup
    ControllerConfig* (*get_config)(ControllerConfigService* self, const char* controller_id);
    ControllerConfig* (*get_config_by_index)(ControllerConfigService* self, int gamepad_index);
    bool (*has_config)(ControllerConfigService* self, const char* controller_id);
    
    // Configuration creation/update
    ControllerConfig* (*create_config)(ControllerConfigService* self, const char* controller_id, const char* name);
    bool (*update_config)(ControllerConfigService* self, const ControllerConfig* config);
    bool (*delete_config)(ControllerConfigService* self, const char* controller_id);
    
    // Calibration
    bool (*start_calibration)(ControllerConfigService* self, const char* controller_id);
    bool (*update_calibration)(ControllerConfigService* self, const char* controller_id, int axis, float value);
    bool (*finish_calibration)(ControllerConfigService* self, const char* controller_id);
    
    // Utility functions
    bool (*apply_config_to_input_service)(ControllerConfigService* self, InputService* input_service, const char* controller_id);
    void (*get_controller_list)(ControllerConfigService* self, char** ids, int* count);
};

// Factory functions
ControllerConfigService* controller_config_service_create(void);
void controller_config_service_destroy(ControllerConfigService* service);

// Utility functions
bool controller_config_load_yaml(const char* path, ControllerConfigDatabase* database);
bool controller_config_save_yaml(const char* path, const ControllerConfigDatabase* database);
void controller_config_apply_defaults(ControllerConfig* config, const ControllerDefaults* defaults);

// Response curve application
float controller_config_apply_curve(float input, ResponseCurveType curve, float exponent);
float controller_config_apply_deadzone(float input, float deadzone);

// Controller ID generation
void controller_config_generate_id(uint16_t vendor_id, uint16_t product_id, char* id_buffer, size_t buffer_size);
bool controller_config_parse_id(const char* id, uint16_t* vendor_id, uint16_t* product_id);

#endif // CONTROLLER_CONFIG_H