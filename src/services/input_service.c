/**
 * @file input_service.c
 * @brief Implementation of high-level input service with action mapping
 */

#include "input_service.h"
#include "input_constants.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define MAX_BINDINGS_PER_ACTION 4
#define MAX_CONTEXT_STACK 8
#define MAX_EVENT_QUEUE 64

// Action state tracking
typedef struct {
    float value;
    float previous_value;
    bool pressed;
    bool was_pressed;
    uint32_t press_timestamp;
    uint32_t release_timestamp;
} ActionState;

// Context-specific bindings
typedef struct {
    InputBinding bindings[INPUT_ACTION_COUNT][MAX_BINDINGS_PER_ACTION];
    int binding_count[INPUT_ACTION_COUNT];
} ContextBindings;

// Internal service data
typedef struct {
    InputHAL* hal;
    
    // Context management
    InputContextID context_stack[MAX_CONTEXT_STACK];
    int context_stack_size;
    
    // Action bindings per context
    ContextBindings contexts[INPUT_CONTEXT_COUNT];
    
    // Current action states
    ActionState action_states[INPUT_ACTION_COUNT];
    
    // Event queue for this frame
    InputEvent event_queue[MAX_EVENT_QUEUE];
    int event_count;
    int event_read_pos;
    
    // Frame timing
    uint32_t current_frame;
    float accumulated_time;
    
    // Configuration
    bool enable_logging;
} InputServiceData;

// Forward declarations
static bool input_service_init(InputService* self, const InputServiceConfig* config);
static void input_service_shutdown(InputService* self);
static void input_service_push_context(InputService* self, InputContextID context);
static void input_service_pop_context(InputService* self);
static InputContextID input_service_get_active_context(InputService* self);
static void input_service_process_frame(InputService* self, float delta_time);
static bool input_service_get_next_event(InputService* self, InputEvent* event);
static float input_service_get_action_value(InputService* self, InputActionID action);
static bool input_service_is_action_pressed(InputService* self, InputActionID action);
static bool input_service_is_action_just_pressed(InputService* self, InputActionID action);
static bool input_service_is_action_just_released(InputService* self, InputActionID action);
static void input_service_bind_action(InputService* self, InputActionID action, InputContextID context, const InputBinding* binding);
static void input_service_clear_action_bindings(InputService* self, InputActionID action, InputContextID context);
static void input_service_save_bindings(InputService* self, const char* profile_name);
static void input_service_load_bindings(InputService* self, const char* profile_name);

// Helper: Check if hardware event matches binding
static bool matches_binding(const HardwareInputEvent* hw_event, const InputBinding* binding) {
    if (hw_event->device != binding->device) {
        return false;
    }
    
    switch (hw_event->device) {
        case INPUT_DEVICE_KEYBOARD:
            return hw_event->data.keyboard.key == binding->binding.keyboard.key &&
                   (binding->binding.keyboard.modifiers == 0 || 
                    (hw_event->data.keyboard.modifiers & binding->binding.keyboard.modifiers) != 0);
            
        case INPUT_DEVICE_MOUSE:
            return (hw_event->data.mouse.buttons & (1 << binding->binding.mouse.button)) != 0;
            
        case INPUT_DEVICE_GAMEPAD:
            return hw_event->data.gamepad.id == binding->binding.gamepad.gamepad_id &&
                   (hw_event->data.gamepad.buttons & (1 << binding->binding.gamepad.button)) != 0;
            
        case INPUT_DEVICE_TOUCH:
        default:
            return false;
    }
}

// Helper: Get value from hardware event for binding
static float get_binding_value(const HardwareInputEvent* hw_event, const InputBinding* binding) {
    float value = 0.0f;
    
    switch (hw_event->device) {
        case INPUT_DEVICE_KEYBOARD:
            value = hw_event->data.keyboard.pressed ? 1.0f : 0.0f;
            break;
            
        case INPUT_DEVICE_MOUSE:
            // Check if this specific button is pressed
            if (hw_event->data.mouse.buttons & (1 << binding->binding.mouse.button)) {
                value = (hw_event->data.mouse.buttons & 0x80) ? 0.0f : 1.0f;  // High bit indicates release
            }
            break;
            
        case INPUT_DEVICE_GAMEPAD:
            // Check if this specific button is pressed
            if (hw_event->data.gamepad.buttons & (1 << binding->binding.gamepad.button)) {
                value = 1.0f;
            } else {
                value = 0.0f;
            }
            // TODO: Add analog axis support using hw_event->data.gamepad.axes[]
            break;
            
        case INPUT_DEVICE_TOUCH:
            // TODO: Implement touch support
            value = 0.0f;
            break;
    }
    
    // Apply scale and inversion
    value *= binding->scale;
    if (binding->invert) {
        value = -value;
    }
    
    return value;
}

// Process a hardware event
static void process_hardware_event(InputServiceData* data, const HardwareInputEvent* hw_event) {
    if (data->context_stack_size == 0) return;
    
    InputContextID active_context = data->context_stack[data->context_stack_size - 1];
    ContextBindings* context = &data->contexts[active_context];
    
    // Check all actions in the active context
    for (int action_id = 0; action_id < INPUT_ACTION_COUNT; action_id++) {
        float new_value = 0.0f;
        bool matched = false;
        
        // Check all bindings for this action
        for (int b = 0; b < context->binding_count[action_id]; b++) {
            InputBinding* binding = &context->bindings[action_id][b];
            
            if (matches_binding(hw_event, binding)) {
                float binding_value = get_binding_value(hw_event, binding);
                
                // For digital inputs, take the max (any pressed = pressed)
                // For analog inputs, sum them (allows WASD + gamepad stick)
                if (binding_value != 0.0f) {
                    if (fabsf(binding_value) > fabsf(new_value)) {
                        new_value = binding_value;
                    }
                    matched = true;
                }
            }
        }
        
        if (matched) {
            ActionState* state = &data->action_states[action_id];
            
            // Update state
            state->previous_value = state->value;
            state->value = new_value;
            state->was_pressed = state->pressed;
            state->pressed = (fabsf(new_value) > 0.1f);  // Dead zone
            
            // Generate events
            if (state->pressed && !state->was_pressed) {
                // Just pressed
                if (data->event_count < MAX_EVENT_QUEUE) {
                    InputEvent* event = &data->event_queue[data->event_count++];
                    event->action = action_id;
                    event->value = new_value;
                    event->just_pressed = true;
                    event->just_released = false;
                    event->timestamp = hw_event->timestamp;
                    state->press_timestamp = hw_event->timestamp;
                    
                    if (data->enable_logging) {
                        printf("🎮 Input: Action %d pressed (value: %.2f)\n", action_id, new_value);
                    }
                }
            } else if (!state->pressed && state->was_pressed) {
                // Just released
                if (data->event_count < MAX_EVENT_QUEUE) {
                    InputEvent* event = &data->event_queue[data->event_count++];
                    event->action = action_id;
                    event->value = 0.0f;
                    event->just_pressed = false;
                    event->just_released = true;
                    event->timestamp = hw_event->timestamp;
                    state->release_timestamp = hw_event->timestamp;
                    
                    if (data->enable_logging) {
                        printf("🎮 Input: Action %d released\n", action_id);
                    }
                }
            } else if (fabsf(state->value - state->previous_value) > 0.01f) {
                // Value changed (analog)
                if (data->event_count < MAX_EVENT_QUEUE) {
                    InputEvent* event = &data->event_queue[data->event_count++];
                    event->action = action_id;
                    event->value = new_value;
                    event->just_pressed = false;
                    event->just_released = false;
                    event->timestamp = hw_event->timestamp;
                }
            }
        }
    }
}

// Implementation functions
static bool input_service_init(InputService* self, const InputServiceConfig* config) {
    if (!self || !self->internal || !config) return false;
    
    InputServiceData* data = (InputServiceData*)self->internal;
    
    if (!config->hal) {
        printf("❌ Input Service: No HAL provided\n");
        return false;
    }
    
    data->hal = config->hal;
    data->enable_logging = config->enable_input_logging;
    
    // Initialize HAL
    if (!data->hal->init(data->hal, data->hal->platform_data)) {
        printf("❌ Input Service: HAL initialization failed\n");
        return false;
    }
    
    // Start with menu context
    data->context_stack[0] = INPUT_CONTEXT_MENU;
    data->context_stack_size = 1;
    
    // Load bindings if path provided
    if (config->bindings_path) {
        input_service_load_bindings(self, config->bindings_path);
    } else {
        // Set up default bindings
        input_service_setup_default_bindings(self);
    }
    
    printf("✅ Input Service initialized\n");
    return true;
}

static void input_service_shutdown(InputService* self) {
    if (!self || !self->internal) return;
    
    InputServiceData* data = (InputServiceData*)self->internal;
    
    if (data && data->hal) {
        data->hal->shutdown(data->hal);
    }
}

static void input_service_push_context(InputService* self, InputContextID context) {
    if (!self || !self->internal) return;
    
    InputServiceData* data = (InputServiceData*)self->internal;
    
    if (data->context_stack_size < MAX_CONTEXT_STACK) {
        data->context_stack[data->context_stack_size++] = context;
        
        if (data->enable_logging) {
            printf("🎮 Input: Pushed context %d (stack size: %d)\n", context, data->context_stack_size);
        }
    }
}

static void input_service_pop_context(InputService* self) {
    if (!self || !self->internal) return;
    
    InputServiceData* data = (InputServiceData*)self->internal;
    
    if (data->context_stack_size > 1) {
        data->context_stack_size--;
        
        if (data->enable_logging) {
            printf("🎮 Input: Popped context (stack size: %d)\n", data->context_stack_size);
        }
    }
}

static InputContextID input_service_get_active_context(InputService* self) {
    if (!self || !self->internal) return INPUT_CONTEXT_MENU;
    
    InputServiceData* data = (InputServiceData*)self->internal;
    
    if (data->context_stack_size > 0) {
        return data->context_stack[data->context_stack_size - 1];
    }
    
    return INPUT_CONTEXT_MENU;
}

static void input_service_process_frame(InputService* self, float delta_time) {
    if (!self || !self->internal) return;
    
    InputServiceData* data = (InputServiceData*)self->internal;
    
    // Clear event queue
    data->event_count = 0;
    data->event_read_pos = 0;
    
    // Update timing
    data->accumulated_time += delta_time;
    data->current_frame++;
    
    // Poll hardware events
    data->hal->poll_events(data->hal);
    
    // Process all hardware events
    HardwareInputEvent hw_event;
    while (data->hal->get_next_event(data->hal, &hw_event)) {
        process_hardware_event(data, &hw_event);
    }
}

static bool input_service_get_next_event(InputService* self, InputEvent* event) {
    if (!self || !self->internal || !event) {
        return false;
    }
    
    InputServiceData* data = (InputServiceData*)self->internal;
    
    if (data->event_read_pos < data->event_count) {
        *event = data->event_queue[data->event_read_pos++];
        return true;
    }
    
    return false;
}

static float input_service_get_action_value(InputService* self, InputActionID action) {
    if (!self || !self->internal) return 0.0f;
    
    InputServiceData* data = (InputServiceData*)self->internal;
    
    if (action < INPUT_ACTION_COUNT) {
        return data->action_states[action].value;
    }
    
    return 0.0f;
}

static bool input_service_is_action_pressed(InputService* self, InputActionID action) {
    if (!self || !self->internal) return false;
    
    InputServiceData* data = (InputServiceData*)self->internal;
    
    if (action < INPUT_ACTION_COUNT) {
        return data->action_states[action].pressed;
    }
    
    return false;
}

static bool input_service_is_action_just_pressed(InputService* self, InputActionID action) {
    if (!self || !self->internal) return false;
    
    InputServiceData* data = (InputServiceData*)self->internal;
    
    if (action < INPUT_ACTION_COUNT) {
        ActionState* state = &data->action_states[action];
        return state->pressed && !state->was_pressed;
    }
    
    return false;
}

static bool input_service_is_action_just_released(InputService* self, InputActionID action) {
    if (!self || !self->internal) return false;
    
    InputServiceData* data = (InputServiceData*)self->internal;
    
    if (action < INPUT_ACTION_COUNT) {
        ActionState* state = &data->action_states[action];
        return !state->pressed && state->was_pressed;
    }
    
    return false;
}

static void input_service_bind_action(InputService* self, InputActionID action, InputContextID context, const InputBinding* binding) {
    if (!self || !self->internal || !binding) return;
    
    InputServiceData* data = (InputServiceData*)self->internal;
    
    if (action >= INPUT_ACTION_COUNT || context >= INPUT_CONTEXT_COUNT) return;
    
    ContextBindings* ctx = &data->contexts[context];
    if (ctx->binding_count[action] < MAX_BINDINGS_PER_ACTION) {
        ctx->bindings[action][ctx->binding_count[action]] = *binding;
        ctx->binding_count[action]++;
        
        if (data->enable_logging) {
            printf("🎮 Input: Bound action %d in context %d\n", action, context);
        }
    }
}

static void input_service_clear_action_bindings(InputService* self, InputActionID action, InputContextID context) {
    if (!self || !self->internal) return;
    
    InputServiceData* data = (InputServiceData*)self->internal;
    
    if (action >= INPUT_ACTION_COUNT || context >= INPUT_CONTEXT_COUNT) return;
    
    data->contexts[context].binding_count[action] = 0;
}

static void input_service_save_bindings(InputService* self, const char* profile_name) {
    // TODO: Implement binding serialization
    (void)self;
    (void)profile_name;
}

static void input_service_load_bindings(InputService* self, const char* profile_name) {
    // TODO: Implement binding deserialization
    (void)self;
    (void)profile_name;
}

// Factory function
InputService* input_service_create(void) {
    InputService* service = (InputService*)calloc(1, sizeof(InputService));
    if (!service) return NULL;
    
    // Allocate internal data
    InputServiceData* data = (InputServiceData*)calloc(1, sizeof(InputServiceData));
    if (!data) {
        free(service);
        return NULL;
    }
    
    // Set up function pointers
    service->init = input_service_init;
    service->shutdown = input_service_shutdown;
    service->push_context = input_service_push_context;
    service->pop_context = input_service_pop_context;
    service->get_active_context = input_service_get_active_context;
    service->process_frame = input_service_process_frame;
    service->get_next_event = input_service_get_next_event;
    service->get_action_value = input_service_get_action_value;
    service->is_action_pressed = input_service_is_action_pressed;
    service->is_action_just_pressed = input_service_is_action_just_pressed;
    service->is_action_just_released = input_service_is_action_just_released;
    service->bind_action = input_service_bind_action;
    service->clear_action_bindings = input_service_clear_action_bindings;
    service->save_bindings = input_service_save_bindings;
    service->load_bindings = input_service_load_bindings;
    service->internal = data;
    
    return service;
}

void input_service_destroy(InputService* service) {
    if (service) {
        if (service->internal) {
            free(service->internal);
        }
        free(service);
    }
}

// Utility macros for cleaner input binding (Sprint 25)
#define KEY_BIND(service, action, context, keycode) do { \
    InputBinding binding = {0}; \
    binding.device = INPUT_DEVICE_KEYBOARD; \
    binding.binding.keyboard.key = (keycode); \
    binding.scale = 1.0f; \
    (service)->bind_action((service), (action), (context), &binding); \
} while(0)

#define PAD_AXIS(service, action, context, axis, scale_factor) do { \
    InputBinding binding = {0}; \
    binding.device = INPUT_DEVICE_GAMEPAD; \
    binding.binding.gamepad.gamepad_id = 0; \
    binding.binding.gamepad.button = (axis); \
    binding.scale = (scale_factor); \
    (service)->bind_action((service), (action), (context), &binding); \
} while(0)

#define PAD_BTN(service, action, context, button) do { \
    InputBinding binding = {0}; \
    binding.device = INPUT_DEVICE_GAMEPAD; \
    binding.binding.gamepad.gamepad_id = 0; \
    binding.binding.gamepad.button = (button); \
    binding.scale = 1.0f; \
    (service)->bind_action((service), (action), (context), &binding); \
} while(0)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// Default bindings setup
void input_service_setup_default_bindings(InputService* service) {
    if (!service) return;
    
    // Menu context bindings (using verbose syntax for non-flight controls)
    InputBinding binding = {0};
    binding.scale = 1.0f;
    
    // UI Up: Arrow Up, W, Gamepad D-Pad Up
    binding.device = INPUT_DEVICE_KEYBOARD;
    binding.binding.keyboard.key = KEY_UP;
    service->bind_action(service, INPUT_ACTION_UI_UP, INPUT_CONTEXT_MENU, &binding);
    
    binding.binding.keyboard.key = KEY_W;
    service->bind_action(service, INPUT_ACTION_UI_UP, INPUT_CONTEXT_MENU, &binding);
    
    // UI Down: Arrow Down, S, Gamepad D-Pad Down
    binding.binding.keyboard.key = KEY_DOWN;
    service->bind_action(service, INPUT_ACTION_UI_DOWN, INPUT_CONTEXT_MENU, &binding);
    
    binding.binding.keyboard.key = KEY_S;
    service->bind_action(service, INPUT_ACTION_UI_DOWN, INPUT_CONTEXT_MENU, &binding);
    
    // UI Confirm: Enter, Space, Gamepad A
    binding.binding.keyboard.key = KEY_ENTER;
    service->bind_action(service, INPUT_ACTION_UI_CONFIRM, INPUT_CONTEXT_MENU, &binding);
    
    binding.binding.keyboard.key = KEY_SPACE;
    service->bind_action(service, INPUT_ACTION_UI_CONFIRM, INPUT_CONTEXT_MENU, &binding);
    
    // UI Cancel: Escape, Gamepad B
    binding.binding.keyboard.key = KEY_ESCAPE;
    service->bind_action(service, INPUT_ACTION_UI_CANCEL, INPUT_CONTEXT_MENU, &binding);
    
    // UI Menu: Tab key (for navigation menu)
    binding.device = INPUT_DEVICE_KEYBOARD;
    binding.binding.keyboard.key = KEY_TAB;
    service->bind_action(service, INPUT_ACTION_UI_MENU, INPUT_CONTEXT_MENU, &binding);
    
    // Mouse bindings for UI
    binding.device = INPUT_DEVICE_MOUSE;
    binding.binding.mouse.button = 0;  // Left click for confirm
    service->bind_action(service, INPUT_ACTION_UI_CONFIRM, INPUT_CONTEXT_MENU, &binding);
    
    // Flight control bindings for gameplay context (Sprint 25)
    // Declarative bindings table for easier maintenance
    static const struct {
        InputActionID action;
        uint32_t keycode;
        const char* description;
    } flight_bindings[] = {
        { INPUT_ACTION_THRUST_FORWARD, KEY_W,    "W - Forward Thrust" },
        { INPUT_ACTION_THRUST_BACK,    KEY_S,    "S - Backward Thrust" },
        { INPUT_ACTION_YAW_LEFT,       KEY_A,    "A - Yaw Left" },
        { INPUT_ACTION_YAW_RIGHT,      KEY_D,    "D - Yaw Right" },
        { INPUT_ACTION_ROLL_LEFT,      KEY_Q,    "Q - Roll Left" },
        { INPUT_ACTION_ROLL_RIGHT,     KEY_E,    "E - Roll Right" },
        { INPUT_ACTION_PITCH_UP,       KEY_UP,   "↑ - Pitch Up" },
        { INPUT_ACTION_PITCH_DOWN,     KEY_DOWN, "↓ - Pitch Down" },
    };
    
    // Apply all flight control bindings
    for (size_t i = 0; i < ARRAY_SIZE(flight_bindings); ++i) {
        KEY_BIND(service, flight_bindings[i].action, INPUT_CONTEXT_GAMEPLAY, flight_bindings[i].keycode);
    }
    
    // Gamepad bindings for flight controls (Sprint 25)
    // Standard Xbox controller layout
    static const struct {
        InputActionID action;
        uint8_t button_or_axis;
        float scale;
        const char* description;
    } gamepad_bindings[] = {
        // Triggers for thrust (RT = forward, LT = backward)
        { INPUT_ACTION_THRUST_FORWARD, 5, 1.0f,  "RT - Forward Thrust" },    // Right Trigger
        { INPUT_ACTION_THRUST_BACK,    4, 1.0f,  "LT - Backward Thrust" },   // Left Trigger
        
        // Right stick for pitch/yaw (primary flight control)
        { INPUT_ACTION_PITCH_UP,       1, -1.0f, "Right Stick Y↑ - Pitch Up" },   // Right stick Y (inverted)
        { INPUT_ACTION_PITCH_DOWN,     1, 1.0f,  "Right Stick Y↓ - Pitch Down" }, // Right stick Y
        { INPUT_ACTION_YAW_LEFT,       0, -1.0f, "Right Stick X← - Yaw Left" },   // Right stick X (inverted)
        { INPUT_ACTION_YAW_RIGHT,      0, 1.0f,  "Right Stick X→ - Yaw Right" },  // Right stick X
        
        // Shoulder buttons for roll
        { INPUT_ACTION_ROLL_LEFT,      6, 1.0f,  "LB - Roll Left" },         // Left Bumper
        { INPUT_ACTION_ROLL_RIGHT,     7, 1.0f,  "RB - Roll Right" },        // Right Bumper
    };
    
    // Apply gamepad bindings
    for (size_t i = 0; i < ARRAY_SIZE(gamepad_bindings); ++i) {
        PAD_AXIS(service, gamepad_bindings[i].action, INPUT_CONTEXT_GAMEPLAY, 
                 gamepad_bindings[i].button_or_axis, gamepad_bindings[i].scale);
    }
    
    printf("✅ Default input bindings configured (keyboard + gamepad flight controls)\n");
}