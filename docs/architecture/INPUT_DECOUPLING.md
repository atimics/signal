# Input System Decoupling Guide

## Overview

This guide explains how to migrate from the current tightly-coupled input system to the new layered architecture.

## Architecture Layers

```
┌─────────────────────────────────┐
│         Game Logic              │
├─────────────────────────────────┤
│      Input Service              │  ← Action mapping, contexts
├─────────────────────────────────┤
│    Hardware Abstraction (HAL)   │  ← Platform abstraction
├─────────────────────────────────┤
│   Platform (Sokol, SDL, etc)   │  ← Platform-specific code
└─────────────────────────────────┘
```

## Migration Steps

### Step 1: Create HAL Implementation for Sokol

```c
// src/hal/input_hal_sokol.c
#include "input_hal.h"
#include "sokol_app.h"

typedef struct SokolInputHAL {
    InputHAL base;
    HardwareInputEvent event_queue[256];
    int queue_head, queue_tail;
} SokolInputHAL;

// Convert Sokol event to HAL event
static void sokol_event_handler(const sapp_event* e) {
    SokolInputHAL* hal = get_sokol_hal();  // Global for now
    
    HardwareInputEvent event = {0};
    event.timestamp = sapp_frame_count();
    
    switch (e->type) {
        case SAPP_EVENTTYPE_KEY_DOWN:
        case SAPP_EVENTTYPE_KEY_UP:
            event.device = INPUT_DEVICE_KEYBOARD;
            event.data.keyboard.key = e->key_code;
            event.data.keyboard.pressed = (e->type == SAPP_EVENTTYPE_KEY_DOWN);
            event.data.keyboard.modifiers = e->modifiers;
            queue_event(hal, &event);
            break;
            
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            event.device = INPUT_DEVICE_MOUSE;
            event.data.mouse.x = e->mouse_x;
            event.data.mouse.y = e->mouse_y;
            event.data.mouse.dx = e->mouse_dx;
            event.data.mouse.dy = e->mouse_dy;
            queue_event(hal, &event);
            break;
    }
}
```

### Step 2: Update Navigation Menu to Use Events

```c
// src/ui_navigation_menu_microui.c
void navigation_menu_process_input(NavigationMenuData* data, InputService* input) {
    InputEvent event;
    
    while (input->get_next_event(&event)) {
        switch (event.action) {
            case INPUT_ACTION_UI_UP:
                if (event.just_pressed && data->selected_index > 0) {
                    data->selected_index--;
                }
                break;
                
            case INPUT_ACTION_UI_DOWN:
                if (event.just_pressed && data->selected_index < data->destination_count - 1) {
                    data->selected_index++;
                }
                break;
                
            case INPUT_ACTION_UI_CONFIRM:
                if (event.just_pressed) {
                    navigation_menu_on_select(data->selected_index, data);
                }
                break;
        }
    }
}
```

### Step 3: Create Compatibility Layer

During migration, maintain compatibility with existing code:

```c
// src/input_compat.c
#include "system/input.h"
#include "services/input_service.h"

static InputService* g_input_service = NULL;

void input_compat_init(InputService* service) {
    g_input_service = service;
}

// Old API implementation using new service
bool input_mapping_just_pressed(uint32_t action) {
    if (!g_input_service) return false;
    
    // Map old action IDs to new ones
    InputActionID new_action = map_legacy_action(action);
    return g_input_service->is_action_just_pressed(new_action);
}
```

## Testing Strategy

### Unit Tests with Mock HAL

```c
// tests/input/test_input_service.c
void test_menu_navigation(void) {
    // Create mock HAL
    InputHAL* hal = input_hal_create_mock();
    
    // Create service
    InputService* service = input_service_create();
    InputServiceConfig config = {
        .hal = hal,
        .bindings_path = NULL,
        .enable_input_logging = true
    };
    service->init(&config);
    
    // Set up test scenario
    service->push_context(INPUT_CONTEXT_MENU);
    
    // Queue test events
    mock_input_queue_key_event(hal, SAPP_KEYCODE_DOWN, true);
    mock_input_queue_key_event(hal, SAPP_KEYCODE_DOWN, false);
    mock_input_queue_key_event(hal, SAPP_KEYCODE_ENTER, true);
    
    // Process frame
    service->process_frame(0.016f);
    
    // Verify events were generated
    InputEvent event;
    TEST_ASSERT_TRUE(service->get_next_event(&event));
    TEST_ASSERT_EQUAL(INPUT_ACTION_UI_DOWN, event.action);
    TEST_ASSERT_TRUE(event.just_pressed);
    
    TEST_ASSERT_TRUE(service->get_next_event(&event));
    TEST_ASSERT_EQUAL(INPUT_ACTION_UI_CONFIRM, event.action);
    TEST_ASSERT_TRUE(event.just_pressed);
    
    // Cleanup
    service->shutdown();
    input_service_destroy(service);
    free(hal);
}
```

## Configuration Format

```yaml
# config/input_bindings.yaml
contexts:
  menu:
    actions:
      ui_up:
        - device: keyboard
          key: UP_ARROW
        - device: keyboard
          key: W
        - device: gamepad
          button: DPAD_UP
      ui_down:
        - device: keyboard
          key: DOWN_ARROW
        - device: keyboard
          key: S
        - device: gamepad
          button: DPAD_DOWN
      ui_confirm:
        - device: keyboard
          key: ENTER
        - device: keyboard
          key: SPACE
        - device: gamepad
          button: A_BUTTON
          
  gameplay:
    actions:
      move_forward:
        - device: keyboard
          key: W
        - device: gamepad
          axis: LEFT_STICK_Y
          scale: -1.0  # Invert Y
```

## Benefits

1. **Testability**: Mock HAL allows deterministic testing
2. **Portability**: Easy to add new platforms
3. **Flexibility**: Runtime binding changes
4. **Debugging**: Input recording/playback
5. **Performance**: Event batching reduces overhead

## Next Steps

1. Implement Sokol HAL (2-3 days)
2. Create Input Service with basic actions (2-3 days)
3. Update navigation menu to use events (1 day)
4. Add compatibility layer (1 day)
5. Write comprehensive tests (2 days)
6. Profile and optimize (1 day)

Total: ~1.5-2 weeks for basic implementation