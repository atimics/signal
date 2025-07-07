/**
 * @file event_handlers.h
 * @brief Standard event handlers for the centralized event router
 */

#ifndef EVENT_HANDLERS_H
#define EVENT_HANDLERS_H

#include "event_router.h"
#include "scene_state.h"
#include "core.h"

// Handler context structures
typedef struct {
    SceneStateManager* scene_state;
    struct World* world;
} SceneEventContext;

typedef struct {
    SceneStateManager* scene_state;
} GlobalEventContext;

// Standard event handlers
bool hal_event_handler(const sapp_event* event, void* context);
bool ui_event_handler(const sapp_event* event, void* context);
bool scene_event_handler(const sapp_event* event, void* context);
bool global_event_handler(const sapp_event* event, void* context);

// Helper functions for registration
void register_standard_event_handlers(EventRouter* router, 
                                     SceneStateManager* scene_state, 
                                     struct World* world);
void unregister_standard_event_handlers(EventRouter* router);

#endif // EVENT_HANDLERS_H