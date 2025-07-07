/**
 * @file event_router.c
 * @brief Implementation of centralized event routing system
 */

#include "event_router.h"
#include <stdio.h>
#include <string.h>

// Global singleton instance
static EventRouter g_event_router = {0};

// ============================================================================
// ROUTER LIFECYCLE
// ============================================================================

void event_router_init(EventRouter* router) {
    if (!router) return;
    
    memset(router, 0, sizeof(EventRouter));
    router->debug_logging = false;
    printf("✅ Event Router initialized\n");
}

void event_router_shutdown(EventRouter* router) {
    if (!router) return;
    
    router->handler_count = 0;
    printf("✅ Event Router shut down\n");
}

// ============================================================================
// HANDLER REGISTRATION
// ============================================================================

bool event_router_register_handler(EventRouter* router, 
                                  EventHandler handler, 
                                  void* context,
                                  EventPriority priority,
                                  const char* name) {
    if (!router || !handler || !name) return false;
    
    if (router->handler_count >= 16) {
        printf("❌ Event Router: Too many handlers registered\n");
        return false;
    }
    
    // Insert handler in priority order (lower priority number = higher priority)
    uint32_t insert_index = router->handler_count;
    for (uint32_t i = 0; i < router->handler_count; i++) {
        if (priority < router->handlers[i].priority) {
            insert_index = i;
            break;
        }
    }
    
    // Shift existing handlers to make room
    for (uint32_t i = router->handler_count; i > insert_index; i--) {
        router->handlers[i] = router->handlers[i - 1];
    }
    
    // Insert new handler
    router->handlers[insert_index] = (EventHandlerRegistration) {
        .handler = handler,
        .context = context,
        .priority = priority,
        .name = name,
        .enabled = true
    };
    
    router->handler_count++;
    
    printf("📝 Event Router: Registered handler '%s' at priority %d\n", name, priority);
    return true;
}

bool event_router_unregister_handler(EventRouter* router, EventHandler handler) {
    if (!router || !handler) return false;
    
    for (uint32_t i = 0; i < router->handler_count; i++) {
        if (router->handlers[i].handler == handler) {
            printf("📝 Event Router: Unregistering handler '%s'\n", router->handlers[i].name);
            
            // Shift remaining handlers down
            for (uint32_t j = i; j < router->handler_count - 1; j++) {
                router->handlers[j] = router->handlers[j + 1];
            }
            
            router->handler_count--;
            return true;
        }
    }
    
    return false;
}

// ============================================================================
// HANDLER MANAGEMENT
// ============================================================================

void event_router_enable_handler(EventRouter* router, const char* name, bool enabled) {
    if (!router || !name) return;
    
    for (uint32_t i = 0; i < router->handler_count; i++) {
        if (strcmp(router->handlers[i].name, name) == 0) {
            router->handlers[i].enabled = enabled;
            printf("📝 Event Router: Handler '%s' %s\n", name, enabled ? "enabled" : "disabled");
            return;
        }
    }
    
    printf("⚠️ Event Router: Handler '%s' not found\n", name);
}

void event_router_set_debug_logging(EventRouter* router, bool enabled) {
    if (!router) return;
    
    router->debug_logging = enabled;
    printf("📝 Event Router: Debug logging %s\n", enabled ? "enabled" : "disabled");
}

// ============================================================================
// MAIN EVENT PROCESSING
// ============================================================================

void event_router_process_event(EventRouter* router, const sapp_event* event) {
    if (!router || !event) return;
    
    if (router->debug_logging) {
        printf("🎯 Event Router: Processing event type %d with %d handlers\n", 
               event->type, router->handler_count);
    }
    
    // Process handlers in priority order
    for (uint32_t i = 0; i < router->handler_count; i++) {
        EventHandlerRegistration* reg = &router->handlers[i];
        
        if (!reg->enabled) {
            if (router->debug_logging) {
                printf("   ⏭️ Skipping disabled handler '%s'\n", reg->name);
            }
            continue;
        }
        
        if (router->debug_logging) {
            printf("   🎯 Trying handler '%s' (priority %d)\n", reg->name, reg->priority);
        }
        
        // Call handler - if it returns true, event is consumed
        if (reg->handler(event, reg->context)) {
            if (router->debug_logging) {
                printf("   ✅ Event consumed by handler '%s'\n", reg->name);
            }
            return;  // Event consumed, stop propagation
        }
        
        if (router->debug_logging) {
            printf("   ⏭️ Handler '%s' did not consume event\n", reg->name);
        }
    }
    
    if (router->debug_logging) {
        printf("   ⚠️ Event not consumed by any handler\n");
    }
}

// ============================================================================
// SINGLETON ACCESS
// ============================================================================

EventRouter* event_router_get_instance(void) {
    return &g_event_router;
}