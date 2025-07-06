/**
 * @file test_mock_hal_ci.c
 * @brief Quick CI test to verify Mock HAL doesn't crash
 */

#include "../../src/hal/input_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(void) {
    printf("Running Mock HAL CI test...\n");
    
    // Create mock HAL
    InputHAL* hal = input_hal_create_mock();
    assert(hal != NULL);
    
    // Initialize
    assert(hal->init(hal, NULL) == true);
    
    // Queue some events
    mock_input_queue_key_event(hal, 65, true);   // A pressed
    mock_input_queue_key_event(hal, 65, false);  // A released
    mock_input_queue_mouse_move(hal, 100.0f, 200.0f);
    
    // Poll events (no-op)
    hal->poll_events(hal);
    
    // Read events back without crashing
    HardwareInputEvent event;
    int event_count = 0;
    while (hal->get_next_event(hal, &event)) {
        event_count++;
        if (event_count > 10) break;  // Safety
    }
    
    printf("Read %d events successfully\n", event_count);
    assert(event_count == 3);  // Should have exactly 3 events
    
    // Test state queries without crashing
    hal->is_key_pressed(hal, 65);
    
    float x, y;
    hal->get_mouse_position(hal, &x, &y);
    printf("Mouse position: %.1f, %.1f\n", x, y);
    
    // Test other functions
    hal->set_mouse_capture(hal, true);
    hal->set_mouse_visible(hal, false);
    hal->vibrate_gamepad(hal, 0, 0.5f, 0.5f);
    
    // Cleanup
    hal->shutdown(hal);
    free(hal);
    
    printf("✅ Mock HAL CI test passed - no crashes!\n");
    return 0;
}