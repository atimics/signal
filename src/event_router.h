/**
 * @file event_router.h
 * @brief Centralized event routing system to eliminate input conflicts
 * 
 * Provides a single, clear path for event handling with proper priority and ownership:
 * 1. HAL layer (input capture)
 * 2. UI layer (captures events it needs)
 * 3. Scene scripts (game-specific handling)
 * 4. Global game events (fallback)
 */

#ifndef EVENT_ROUTER_H
#define EVENT_ROUTER_H

#include "sokol_app.h"
#include "core.h"

// Event handling priorities (lower number = higher priority)
typedef enum {
    EVENT_PRIORITY_HAL = 0,       // Input HAL always gets events first
    EVENT_PRIORITY_UI = 1,        // UI can capture events before game logic
    EVENT_PRIORITY_SCENE = 2,     // Scene-specific handling
    EVENT_PRIORITY_GLOBAL = 3     // Global game events (fallback)
} EventPriority;

// Event handler function signature
// Returns true if event was handled (stops propagation)
typedef bool (*EventHandler)(const sapp_event* event, void* context);

// Event handler registration
typedef struct {
    EventHandler handler;
    void* context;
    EventPriority priority;
    const char* name;  // For debugging
    bool enabled;
} EventHandlerRegistration;

// Router state
typedef struct {
    EventHandlerRegistration handlers[16];  // Max 16 handlers
    uint32_t handler_count;
    bool debug_logging;
} EventRouter;

// Router lifecycle
void event_router_init(EventRouter* router);
void event_router_shutdown(EventRouter* router);

// Handler registration
bool event_router_register_handler(EventRouter* router, 
                                 EventHandler handler, 
                                 void* context,
                                 EventPriority priority,
                                 const char* name);

bool event_router_unregister_handler(EventRouter* router, EventHandler handler);

// Handler management
void event_router_enable_handler(EventRouter* router, const char* name, bool enabled);
void event_router_set_debug_logging(EventRouter* router, bool enabled);

// Main event processing
void event_router_process_event(EventRouter* router, const sapp_event* event);

// Get singleton instance
EventRouter* event_router_get_instance(void);

#endif // EVENT_ROUTER_H