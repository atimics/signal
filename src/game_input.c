/**
 * @file game_input.c
 * @brief Implementation of game-level input management
 */

#include "game_input.h"
#include "services/input_service.h"
#include "hal/input_hal.h"
#include "scene_state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Feature flag - can be set via environment variable or config
#define USE_NEW_INPUT_SYSTEM_ENV "CGAME_USE_NEW_INPUT"


// Global state
static struct {
    InputService* service;
    InputHAL* hal;
    bool initialized;
    bool new_system_enabled;
} g_game_input = {0};

// Check if new input system should be enabled
static bool check_new_input_enabled(void) {
    // Always use new input system - legacy system deprecated
    return true;
}

bool game_input_init(void) {
    if (g_game_input.initialized) {
        printf("⚠️  Game input already initialized\n");
        return true;
    }
    
    // Check if new system should be enabled
    g_game_input.new_system_enabled = check_new_input_enabled();
    
    printf("🎮 Initializing input system...\n");
    
    // Create HAL (Sokol implementation)
    g_game_input.hal = input_hal_create_sokol();
    if (!g_game_input.hal) {
        printf("❌ Failed to create Sokol input HAL\n");
        return false;
    }
    
    // Create Input Service
    g_game_input.service = input_service_create();
    if (!g_game_input.service) {
        printf("❌ Failed to create input service\n");
        free(g_game_input.hal);
        g_game_input.hal = NULL;
        return false;
    }
    
    // Configure and initialize service
    InputServiceConfig config = {
        .hal = g_game_input.hal,
        .bindings_path = NULL,  // Use default bindings
        .enable_input_logging = false  // Set to true for debugging
    };
    
    if (!g_game_input.service->init(g_game_input.service, &config)) {
        printf("❌ Failed to initialize input service\n");
        input_service_destroy(g_game_input.service);
        free(g_game_input.hal);
        g_game_input.service = NULL;
        g_game_input.hal = NULL;
        return false;
    }
    
    g_game_input.initialized = true;
    printf("✅ Input system initialized successfully\n");
    
    return true;
}

void game_input_shutdown(void) {
    if (!g_game_input.initialized) {
        return;
    }
    
    printf("🎮 Shutting down input system...\n");
    
    
    // Shutdown service
    if (g_game_input.service) {
        g_game_input.service->shutdown(g_game_input.service);
        input_service_destroy(g_game_input.service);
        g_game_input.service = NULL;
    }
    
    // Cleanup HAL
    if (g_game_input.hal) {
        free(g_game_input.hal);
        g_game_input.hal = NULL;
    }
    
    g_game_input.initialized = false;
    printf("✅ Input system shutdown complete\n");
}

void game_input_process_frame(float delta_time) {
    if (!g_game_input.initialized || !g_game_input.service) {
        return;
    }
    
    // Process input through the service
    g_game_input.service->process_frame(g_game_input.service, delta_time);
}

bool game_input_is_new_system_enabled(void) {
    return g_game_input.new_system_enabled;
}

InputService* game_input_get_service(void) {
    return g_game_input.service;
}

