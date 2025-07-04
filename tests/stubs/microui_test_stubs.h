/**
 * @file microui_test_stubs.h
 * @brief Test helpers for MicroUI unit testing
 */

#ifndef MICROUI_TEST_STUBS_H
#define MICROUI_TEST_STUBS_H

// Test state management
void microui_test_reset(void);
int microui_test_get_command_count(void);
int microui_test_get_vertex_count(void);
int microui_test_get_window_count(void);
int microui_test_get_button_clicks(void);

#endif // MICROUI_TEST_STUBS_H