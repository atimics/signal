#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include <stdbool.h>
#include "math3d.h"

// Forward declarations to avoid circular dependencies
struct Entity;
struct Universe;

// AI task types
typedef enum {
    AI_TASK_DIALOG,
    AI_TASK_COMBAT,
    AI_TASK_NAVIGATION,
    AI_TASK_BEHAVIOR_SCRIPT,
    AI_TASK_TRADE,
    AI_TASK_SOCIAL,
    AI_TASK_CREATIVE,
    AI_TASK_REACTION
} AITaskType;

// Global AI engine instance (for dialog system access)
extern struct LlamaInference* ai_get_llama_engine(void);

// AI Context (like your location + sensors approach)
struct AIContext {
    // Spatial awareness
    Vector3D position;
    Vector3D velocity;
    float view_distance;
    
    // Sensor data
    int nearby_entities_count;
    struct Entity** nearby_entities;
    
    // Current state
    char* current_goal;
    char* personality_prompt;
    char* memory_context;
    
    // Task-specific context
    char* task_prompt;
    char* expected_output_format;
    
    // Performance metrics
    float response_time_requirement;  // For LOD system
    int context_token_limit;
};

// AI Task Queue Entry
struct AITask {
    uint32_t id;
    AITaskType type;
    struct Entity* entity;
    struct AIContext* context;
    char* prompt;
    void (*callback)(struct Entity* entity, char* ai_response);
    void (*response_callback)(struct Entity* entity, char* ai_response);
    float priority;
    float deadline;
    float creation_time;
    struct AITask* next;
};

// Main AI Engine (single instance for all entities)
struct AIEngine {
    // Model state
    void* model_handle;           // Gemma/Llama model
    char* model_path;
    bool is_loaded;
    
    // Task queue management
    struct AITask* high_priority_queue;  // Chatbots, real-time interactions
    struct AITask* medium_priority_queue; // Combat, navigation
    struct AITask* low_priority_queue;   // Background scripts, planning
    
    // Performance tracking
    float average_inference_time;
    int tasks_processed_per_second;
    int total_entities_managed;
    float total_runtime;
    uint32_t next_task_id;
    
    // Memory management
    char* shared_context_buffer;
    int max_context_size;
    
    // LOD system
    float lod_distance_threshold;
    int max_concurrent_tasks;
};

// AI Engine API
bool ai_engine_init(struct AIEngine* engine, const char* model_path);
void ai_engine_shutdown(struct AIEngine* engine);

// Task management
void ai_engine_submit_task(struct AIEngine* engine, struct AITask* task);
void ai_engine_process_tasks(struct AIEngine* engine, float deltaTime);

// Context building (inspired by your sensor + location approach)
struct AIContext* ai_build_context(struct Entity* entity, struct Universe* universe);
void ai_context_destroy(struct AIContext* context);

// LOD system
float ai_calculate_priority(struct Entity* entity, Vector3D player_position);
bool ai_should_process_entity(struct Entity* entity, float distance, AITaskType task_type);

// Convenience functions for common tasks
void ai_request_dialog(struct AIEngine* engine, struct Entity* entity, const char* player_message, 
                      void (*response_callback)(struct Entity*, char*));
void ai_request_behavior_update(struct AIEngine* engine, struct Entity* entity);
void ai_request_navigation_decision(struct AIEngine* engine, struct Entity* entity, Vector3D target);
void ai_request_reaction(struct AIEngine* engine, struct Entity* entity, struct Entity* player, float distance);

// Internal wrapper functions
void ai_reaction_response_wrapper(struct Entity* entity, char* response);

#endif
