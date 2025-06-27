#include "cgame.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Real AI inference using Llama
static struct LlamaInference g_llama_engine = {0};

// Accessor function for dialog system
struct LlamaInference* ai_get_llama_engine(void) {
    return &g_llama_engine;
}

bool ai_engine_init(struct AIEngine* engine, const char* model_path) {
    if (!engine) return false;
    
    printf("🧠 Initializing AI Engine with neural model: %s\n", model_path);
    
    // Initialize engine structure
    memset(engine, 0, sizeof(struct AIEngine));
    
    // Initialize real Llama inference engine
    if (!llama_init(&g_llama_engine, model_path, 2048)) {
        printf("❌ Failed to initialize Llama inference engine\n");
        return false;
    }
    
    engine->model_handle = &g_llama_engine;
    engine->model_path = strdup(model_path);
    engine->is_loaded = llama_is_ready(&g_llama_engine);
    
    // Initialize performance settings
    engine->max_context_size = 2048;
    engine->lod_distance_threshold = 500.0f;
    engine->max_concurrent_tasks = 5; // Reduced for real AI processing
    
    // Allocate shared context buffer
    engine->shared_context_buffer = malloc(engine->max_context_size);
    
    printf("✅ AI Engine with neural inference initialized successfully\n");
    return true;
}

void ai_engine_shutdown(struct AIEngine* engine) {
    if (!engine) return;
    
    printf("🔄 Shutting down AI Engine\n");
    
    // Free all pending tasks
    struct AITask* task = engine->high_priority_queue;
    while (task) {
        struct AITask* next = task->next;
        if (task->prompt) free(task->prompt);
        free(task);
        task = next;
    }
    
    // Shutdown Llama inference engine
    llama_shutdown(&g_llama_engine);
    
    // Free resources
    if (engine->model_path) free(engine->model_path);
    if (engine->shared_context_buffer) free(engine->shared_context_buffer);
    
    memset(engine, 0, sizeof(struct AIEngine));
    printf("✅ AI Engine shutdown complete\n");
}

void ai_engine_submit_task(struct AIEngine* engine, struct AITask* task) {
    if (!engine || !task) return;
    
    // Add to appropriate priority queue
    struct AITask** queue = NULL;
    switch (task->type) {
        case AI_TASK_DIALOG:
            queue = &engine->high_priority_queue;
            break;
        case AI_TASK_COMBAT:
        case AI_TASK_NAVIGATION:
            queue = &engine->medium_priority_queue;
            break;
        default:
            queue = &engine->low_priority_queue;
            break;
    }
    
    // Insert at head of queue (simple LIFO for now)
    task->next = *queue;
    *queue = task;
    
    printf("📝 AI Task submitted: Type=%d, Entity=%s\n", 
           task->type, task->entity ? task->entity->name : "Unknown");
}

// Context-aware prompt building (inspired by your CosyWorld sensor + location approach)
static char* build_context_aware_prompt(struct AITask* task, struct AIEngine* engine) {
    if (!task || !task->entity) return strdup(task->prompt ? task->prompt : "");
    (void)engine; // Suppress unused parameter warning
    
    // Suppress unused parameter warning for now (engine may be used for shared context later)
    (void)engine;
    
    // Build rich context like CosyWorld's sensor system
    char* context_buffer = malloc(4096);
    snprintf(context_buffer, 4096,
        "You are %s, a %s in a 3D space combat simulation.\n"
        "Position: (%.1f, %.1f, %.1f)\n"
        "Velocity: (%.1f, %.1f, %.1f)\n"
        "Current State: %s\n"
        "Health: %.1f%% | Energy: %.1f%% | Fuel: %.1f%%\n",
        task->entity->name ? task->entity->name : "Unknown Entity",
        task->entity->type == ENTITY_AI_FIGHTER ? "Fighter Pilot" :
        task->entity->type == ENTITY_AI_TRADER ? "Merchant" :
        task->entity->type == ENTITY_AI_EXPLORER ? "Explorer" : "AI Entity",
        task->entity->position.x, task->entity->position.y, task->entity->position.z,
        task->entity->velocity.x, task->entity->velocity.y, task->entity->velocity.z,
        task->entity->current_state == ENTITY_STATE_IDLE ? "Idle" :
        task->entity->current_state == ENTITY_STATE_PATROLLING ? "Patrolling" :
        task->entity->current_state == ENTITY_STATE_PURSUING ? "In Pursuit" :
        task->entity->current_state == ENTITY_STATE_FLEEING ? "Fleeing" :
        task->entity->current_state == ENTITY_STATE_COMMUNICATING ? "In Dialog" : "Unknown",
        task->entity->health, task->entity->energy, task->entity->fuel
    );
    
    // Add personality context if available
    if (task->entity->personality && task->entity->personality->base_prompt) {
        strncat(context_buffer, "\nPersonality: ", 4095 - strlen(context_buffer));
        strncat(context_buffer, task->entity->personality->base_prompt, 4095 - strlen(context_buffer));
    }
    
    // Add nearby entities context (like CosyWorld's spatial awareness)
    if (task->context && task->context->nearby_entities_count > 0) {
        strncat(context_buffer, "\nNearby Entities: ", 4095 - strlen(context_buffer));
        for (int i = 0; i < task->context->nearby_entities_count && i < 3; i++) {
            struct Entity* nearby = task->context->nearby_entities[i];
            char nearby_info[256];
            snprintf(nearby_info, sizeof(nearby_info), "%s at (%.1f, %.1f, %.1f); ",
                nearby->name ? nearby->name : "Unknown",
                nearby->position.x, nearby->position.y, nearby->position.z);
            strncat(context_buffer, nearby_info, 4095 - strlen(context_buffer));
        }
    }
    
    // Add the specific task prompt
    strncat(context_buffer, "\n\nTask: ", 4095 - strlen(context_buffer));
    strncat(context_buffer, task->prompt ? task->prompt : "Respond appropriately.", 4095 - strlen(context_buffer));
    strncat(context_buffer, "\n\nResponse: ", 4095 - strlen(context_buffer));
    
    return context_buffer;
}

void ai_engine_process_tasks(struct AIEngine* engine, float deltaTime) {
    if (!engine || !llama_is_ready(&g_llama_engine)) return;
    
    clock_t start_time = clock();
    int tasks_processed = 0;
    
    // Process high priority tasks first (real-time dialog)
    struct AITask* task = engine->high_priority_queue;
    while (task && tasks_processed < engine->max_concurrent_tasks) {
        struct AITask* next = task->next;
        
        // Build context-aware prompt
        char* full_prompt = build_context_aware_prompt(task, engine);
        
        // Real neural inference using Llama
        printf("🧠 Processing AI task: %s\n", 
               task->type == AI_TASK_DIALOG ? "Dialog" :
               task->type == AI_TASK_COMBAT ? "Combat" :
               task->type == AI_TASK_NAVIGATION ? "Navigation" :
               task->type == AI_TASK_BEHAVIOR_SCRIPT ? "Behavior" : "Unknown");
        
        char* ai_response = llama_generate(&g_llama_engine, full_prompt, 150);
        
        // Call the callback with the AI response
        if (task->callback && task->entity && ai_response) {
            task->callback(task->entity, ai_response);
        }
        
        // Cleanup
        if (task->prompt) free(task->prompt);
        if (full_prompt) free(full_prompt);
        if (ai_response) free(ai_response);
        free(task);
        
        task = next;
        tasks_processed++;
    }
    engine->high_priority_queue = task;
    
    // Process medium priority if we have cycles left
    if (tasks_processed < engine->max_concurrent_tasks) {
        task = engine->medium_priority_queue;
        while (task && tasks_processed < engine->max_concurrent_tasks) {
            struct AITask* next = task->next;
            
            // Build context-aware prompt
            char* full_prompt = build_context_aware_prompt(task, engine);
            
            // Real neural inference for medium priority tasks
            char* ai_response = llama_generate(&g_llama_engine, full_prompt, 100);
            
            if (task->callback && task->entity && ai_response) {
                task->callback(task->entity, ai_response);
            }
            
            if (task->prompt) free(task->prompt);
            if (full_prompt) free(full_prompt);
            if (ai_response) free(ai_response);
            free(task);
            
            task = next;
            tasks_processed++;
        }
        engine->medium_priority_queue = task;
    }
    
    // Update performance metrics
    clock_t end_time = clock();
    float processing_time = ((float)(end_time - start_time)) / CLOCKS_PER_SEC;
    engine->average_inference_time = llama_get_last_inference_time(&g_llama_engine);
    engine->tasks_processed_per_second = tasks_processed / deltaTime;
    
    if (tasks_processed > 0) {
        printf("� Neural AI processed %d tasks in %.3fms (avg: %.3fms per task)\n", 
               tasks_processed, processing_time * 1000, 
               engine->average_inference_time * 1000);
    }
}

struct AIContext* ai_build_context(struct Entity* entity, struct Universe* universe) {
    if (!entity || !universe) return NULL;
    
    struct AIContext* context = malloc(sizeof(struct AIContext));
    memset(context, 0, sizeof(struct AIContext));
    
    // Copy spatial data
    context->position = entity->position;
    context->velocity = entity->velocity;
    context->view_distance = entity->sensors ? entity->sensors->visual_range : 100.0f;
    
    // Find nearby entities for context
    struct Entity* nearby[10];
    context->nearby_entities_count = universe_find_entities_near(
        universe, entity->position, context->view_distance, nearby, 10);
    
    if (context->nearby_entities_count > 0) {
        context->nearby_entities = malloc(sizeof(struct Entity*) * context->nearby_entities_count);
        memcpy(context->nearby_entities, nearby, sizeof(struct Entity*) * context->nearby_entities_count);
    }
    
    // Set personality context
    if (entity->personality) {
        context->personality_prompt = strdup(entity->personality->base_prompt);
    }
    
    // Performance settings based on distance to player
    context->response_time_requirement = entity->distance_to_player < 100.0f ? 50.0f : 500.0f; // ms
    context->context_token_limit = entity->distance_to_player < 100.0f ? 1024 : 256;
    
    return context;
}

void ai_context_destroy(struct AIContext* context) {
    if (!context) return;
    
    if (context->nearby_entities) free(context->nearby_entities);
    if (context->current_goal) free(context->current_goal);
    if (context->personality_prompt) free(context->personality_prompt);
    if (context->memory_context) free(context->memory_context);
    if (context->task_prompt) free(context->task_prompt);
    if (context->expected_output_format) free(context->expected_output_format);
    
    free(context);
}

float ai_calculate_priority(struct Entity* entity, Vector3D player_position) {
    if (!entity) return 0.0f;
    
    // Calculate distance to player
    Vector3D diff = {
        entity->position.x - player_position.x,
        entity->position.y - player_position.y,
        entity->position.z - player_position.z
    };
    float distance = sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
    
    // Base priority inversely proportional to distance
    float priority = 1.0f / (1.0f + distance * 0.01f);
    
    // Boost priority for entities player is interacting with
    if (entity->last_message_received && strstr(entity->last_message_received, "player")) {
        priority *= 10.0f;
    }
    
    // Boost priority for combat situations
    if (entity->current_state == ENTITY_STATE_PURSUING || 
        entity->current_state == ENTITY_STATE_FLEEING) {
        priority *= 5.0f;
    }
    
    return priority;
}

bool ai_should_process_entity(struct Entity* entity, float distance, AITaskType task_type) {
    if (!entity) return false;
    
    // Always process high-priority tasks regardless of distance
    if (task_type == AI_TASK_DIALOG) {
        return true;
    }
    
    // Distance-based LOD
    if (distance < 100.0f) return true;   // Full processing
    if (distance < 500.0f) {              // Medium processing
        return task_type == AI_TASK_COMBAT || task_type == AI_TASK_NAVIGATION;
    }
    if (distance < 2000.0f) {             // Low processing
        return task_type == AI_TASK_BEHAVIOR_SCRIPT;
    }
    
    return false; // Too far, no processing
}

void ai_request_dialog(struct AIEngine* engine, struct Entity* entity, const char* player_message, 
                      void (*response_callback)(struct Entity*, char*)) {
    if (!engine || !entity) return;
    
    printf("💬 Processing dialog request for %s\n", entity->name ? entity->name : "Unknown Entity");
    
    struct AITask* task = malloc(sizeof(struct AITask));
    memset(task, 0, sizeof(struct AITask));
    
    task->type = AI_TASK_DIALOG;
    task->entity = entity;
    task->priority = 10.0f; // Highest priority for responsive dialog
    task->deadline = 0.2f;  // 200ms deadline for real-time feel
    task->callback = response_callback;
    
    // Build immersive dialog prompt (like CosyWorld's character consistency)
    char prompt[1024];
    snprintf(prompt, sizeof(prompt), 
        "DIALOG_REQUEST: Player says to %s: '%s'\n"
        "Respond in character as %s. Keep under 120 characters.",
        entity->name ? entity->name : "this entity",
        player_message ? player_message : "Hello",
        entity->name ? entity->name : "a space pilot");
    task->prompt = strdup(prompt);
    
    ai_engine_submit_task(engine, task);
}

void ai_request_behavior_update(struct AIEngine* engine, struct Entity* entity) {
    if (!engine || !entity) return;
    
    struct AITask* task = malloc(sizeof(struct AITask));
    memset(task, 0, sizeof(struct AITask));
    
    task->type = AI_TASK_BEHAVIOR_SCRIPT;
    task->entity = entity;
    task->priority = 1.0f;
    task->deadline = 10.0f; // 10 second deadline for behavior updates
    
    // Build behavior prompt with rich context
    char prompt[1024];
    snprintf(prompt, sizeof(prompt), 
        "BEHAVIOR_UPDATE: %s at (%.1f, %.1f, %.1f)\n"
        "Current state: %s | Health: %.0f%% | Energy: %.0f%%\n"
        "Generate next action for this %s pilot:",
        entity->name ? entity->name : "Pilot",
        entity->position.x, entity->position.y, entity->position.z,
        entity->current_state == ENTITY_STATE_IDLE ? "Idle" :
        entity->current_state == ENTITY_STATE_PATROLLING ? "Patrolling" : "Active",
        entity->health, entity->energy,
        entity->type == ENTITY_AI_FIGHTER ? "fighter" :
        entity->type == ENTITY_AI_TRADER ? "merchant" : "civilian");
    task->prompt = strdup(prompt);
    
    ai_engine_submit_task(engine, task);
}

void ai_request_navigation_decision(struct AIEngine* engine, struct Entity* entity, Vector3D target) {
    if (!engine || !entity) return;
    
    struct AITask* task = malloc(sizeof(struct AITask));
    memset(task, 0, sizeof(struct AITask));
    
    task->type = AI_TASK_NAVIGATION;
    task->entity = entity;
    task->priority = 5.0f;
    task->deadline = 2.0f; // 2 second deadline for navigation
    
    // Calculate distance and bearing
    Vector3D diff = {target.x - entity->position.x, 
                     target.y - entity->position.y, 
                     target.z - entity->position.z};
    float distance = sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
    
    char prompt[1024];
    snprintf(prompt, sizeof(prompt), 
        "NAVIGATION_REQUEST: %s needs to travel %.1f units\n"
        "From: (%.1f, %.1f, %.1f) To: (%.1f, %.1f, %.1f)\n"
        "Current velocity: (%.1f, %.1f, %.1f)\n"
        "Fuel: %.0f%% | Energy: %.0f%%\n"
        "Plan optimal route and speed:",
        entity->name ? entity->name : "Pilot",
        distance,
        entity->position.x, entity->position.y, entity->position.z,
        target.x, target.y, target.z,
        entity->velocity.x, entity->velocity.y, entity->velocity.z,
        entity->fuel, entity->energy);
    task->prompt = strdup(prompt);
    
    ai_engine_submit_task(engine, task);
}
