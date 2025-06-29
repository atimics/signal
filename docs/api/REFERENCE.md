# API Reference - CGame ECS Engine

This document provides comprehensive API documentation for the CGame Entity-Component-System engine.

## Table of Contents

1. [Core ECS API](#core-ecs-api)
2. [Component API](#component-api)
3. [System API](#system-api)
4. [Asset Management API](#asset-management-api)
5. [Data-Driven API](#data-driven-api)
6. [Rendering API](#rendering-api)

---

## Core ECS API

The core ECS module (`core.h/c`) provides the foundation for entity and component management.

### World Management

#### `bool world_init(struct World* world)`
```c
bool world_init(struct World* world)
```
Initializes a new world instance. Must be called before any entity operations.
- **Parameters**: `world` - Pointer to world structure to initialize
- **Returns**: `true` on success, `false` on failure

#### `void world_destroy(struct World* world)`
```c
void world_destroy(struct World* world)
```
Safely destroys a world and all its entities and components.
- **Parameters**: `world` - Pointer to world to destroy

#### `void world_update(struct World* world, float delta_time)`
```c
void world_update(struct World* world, float delta_time)
```
Updates world timing information. Called once per frame.
- **Parameters**: 
  - `world` - World to update
  - `delta_time` - Time since last frame in seconds

### Entity Management

#### `EntityID entity_create(struct World* world)`
```c
EntityID entity_create(struct World* world)
```
Creates a new entity with a unique ID.
- **Parameters**: `world` - World to create entity in
- **Returns**: Unique EntityID, or `INVALID_ENTITY` on failure

#### `void entity_destroy(struct World* world, EntityID entity_id)`
```c
void entity_destroy(struct World* world, EntityID entity_id)
```
Destroys an entity and removes all its components.
- **Parameters**:
  - `world` - World containing the entity
  - `entity_id` - ID of entity to destroy

#### `bool entity_add_component(struct World* world, EntityID entity_id, ComponentType component_type)`
```c
bool entity_add_component(struct World* world, EntityID entity_id, ComponentType component_type)
```
Adds a component to an entity. Can be combined with bitwise OR for multiple components.
- **Parameters**:
  - `world` - World containing the entity
  - `entity_id` - Target entity ID
  - `component_type` - Component type(s) to add (can be combined)
- **Returns**: `true` on success, `false` on failure

#### `void entity_remove_component(struct World* world, EntityID entity_id, ComponentType component_type)`
```c
void entity_remove_component(struct World* world, EntityID entity_id, ComponentType component_type)
```
Removes a component from an entity.
- **Parameters**:
  - `world` - World containing the entity
  - `entity_id` - Target entity ID
  - `component_type` - Component type to remove

---

## Component API

Component accessor functions provide safe access to entity components.

### Component Accessors

#### `struct Transform* entity_get_transform(struct World* world, EntityID entity_id)`
```c
struct Transform* entity_get_transform(struct World* world, EntityID entity_id)
```
Gets transform component for position, rotation, and scale.
- **Returns**: Pointer to Transform component, or `NULL` if not present

#### `struct Physics* entity_get_physics(struct World* world, EntityID entity_id)`
```c
struct Physics* entity_get_physics(struct World* world, EntityID entity_id)
```
Gets physics component for velocity and forces.
- **Returns**: Pointer to Physics component, or `NULL` if not present

#### `struct Collision* entity_get_collision(struct World* world, EntityID entity_id)`
```c
struct Collision* entity_get_collision(struct World* world, EntityID entity_id)
```
Gets collision component for collision detection.
- **Returns**: Pointer to Collision component, or `NULL` if not present

#### `struct AI* entity_get_ai(struct World* world, EntityID entity_id)`
```c
struct AI* entity_get_ai(struct World* world, EntityID entity_id)
```
Gets AI component for autonomous behavior.
- **Returns**: Pointer to AI component, or `NULL` if not present

#### `struct Renderable* entity_get_renderable(struct World* world, EntityID entity_id)`
```c
struct Renderable* entity_get_renderable(struct World* world, EntityID entity_id)
```
Gets renderable component for visual representation.
- **Returns**: Pointer to Renderable component, or `NULL` if not present

#### `struct Player* entity_get_player(struct World* world, EntityID entity_id)`
```c
struct Player* entity_get_player(struct World* world, EntityID entity_id)
```
Gets player component for input handling.
- **Returns**: Pointer to Player component, or `NULL` if not present

### Component Data Structures

#### `struct Transform`
```c
struct Transform {
    Vector3 position;     // World position
    Quaternion rotation;  // Orientation quaternion
    Vector3 scale;        // Scale factors
    bool dirty;           // Needs matrix recalculation
};
```

#### `struct Physics`
```c
struct Physics {
    Vector3 velocity;      // Current velocity
    Vector3 acceleration;  // Current acceleration
    float mass;            // Object mass
    float drag;            // Drag coefficient (0.0-1.0)
    bool kinematic;        // Not affected by forces
};
```

#### `struct Collision`
```c
struct Collision {
    float radius;          // Collision sphere radius
    uint32_t layer_mask;   // Collision layer bitmask
    bool is_trigger;       // Ghost collision
    Vector3 offset;        // Offset from transform position
};
```

#### `struct AI`
```c
struct AI {
    int state;             // Current AI state
    float update_frequency; // How often to update (Hz)
    float decision_timer;   // Time of last decision
    float reaction_cooldown; // Prevent spam reactions
    Vector3 target_position; // Current movement target
};
```

---

## System API

Systems process components and implement game logic (`systems.h/c`).

### System Scheduler

#### `bool system_scheduler_init(struct SystemScheduler* scheduler)`
```c
bool system_scheduler_init(struct SystemScheduler* scheduler)
```
Initializes the system scheduler with default system configurations.

#### `void system_scheduler_update(struct SystemScheduler* scheduler, struct World* world, float delta_time)`
```c
void system_scheduler_update(struct SystemScheduler* scheduler, struct World* world, float delta_time)
```
Updates all systems based on their scheduled frequencies.

### Core Systems

#### `void physics_system_update(struct World* world, float delta_time)`
```c
void physics_system_update(struct World* world, float delta_time)
```
Updates physics simulation for all entities with Transform and Physics components.
- **Frequency**: 60 FPS
- **Purpose**: Apply forces, update positions, handle basic movement

#### `void collision_system_update(struct World* world, float delta_time)`
```c
void collision_system_update(struct World* world, float delta_time)
```
Performs collision detection and response.
- **Frequency**: 20 FPS
- **Purpose**: Detect collisions, generate collision events

#### `void ai_system_update(struct World* world, float delta_time)`
```c
void ai_system_update(struct World* world, float delta_time)
```
Updates AI behavior with level-of-detail optimization.
- **Frequency**: 2-10 FPS (distance-based)
- **Purpose**: AI decision making, pathfinding, behavior execution

---

## Asset Management API

Asset management system (`assets.h/c`) handles loading and caching of game resources.

#### `bool assets_init(struct AssetManager* manager)`
```c
bool assets_init(struct AssetManager* manager)
```
Initializes the asset management system.

#### `struct Mesh* assets_load_mesh(struct AssetManager* manager, const char* name)`
```c
struct Mesh* assets_load_mesh(struct AssetManager* manager, const char* name)
```
Loads an OBJ mesh file from the assets directory.
- **Parameters**: 
  - `manager` - Asset manager instance
  - `name` - Mesh name (without .mesh extension)
- **Returns**: Pointer to loaded mesh, or `NULL` on failure

#### `struct Texture* assets_load_texture(struct AssetManager* manager, const char* name)`
```c
struct Texture* assets_load_texture(struct AssetManager* manager, const char* name)
```
Loads a texture file (PNG/JPG) from the assets directory.

---

## Data-Driven API

Data system (`data.h/c`) enables entity and scene creation from external files.

#### `bool data_load_templates(const char* filename)`
```c
bool data_load_templates(const char* filename)
```
Loads entity templates from a data file.
- **Parameters**: `filename` - Path to templates file (e.g., "data/templates/entities.txt")

#### `EntityID data_create_entity_from_template(struct World* world, const char* template_name)`
```c
EntityID data_create_entity_from_template(struct World* world, const char* template_name)
```
Creates an entity using a predefined template.
- **Parameters**:
  - `world` - Target world
  - `template_name` - Name of template to instantiate

#### `bool data_load_scene(struct World* world, const char* scene_name)`
```c
bool data_load_scene(struct World* world, const char* scene_name)
```
Loads a complete scene with multiple entities.

---

## Rendering API

3D rendering system (`render_3d.c`, `render.h`) handles visual output.

#### `bool render_3d_init(struct RenderConfig* config)`
```c
bool render_3d_init(struct RenderConfig* config)
```
Initializes the 3D rendering system with SDL2.

#### `void render_3d_frame(struct World* world, struct RenderConfig* config)`
```c
void render_3d_frame(struct World* world, struct RenderConfig* config)
```
Renders one frame of all visible entities.

#### `void render_3d_debug_draw_collision(Vector3 position, float radius)`
```c
void render_3d_debug_draw_collision(Vector3 position, float radius)
```
Draws debug visualization for collision spheres.

---

## Entity Factory Functions

Common entity creation patterns (defined in `test.c`):

#### `EntityID create_player(struct World* world, Vector3 position)`
```c
EntityID create_player(struct World* world, Vector3 position)
```
Creates a player ship with full component set.

#### `EntityID create_ai_ship(struct World* world, Vector3 position, const char* type)`
```c
EntityID create_ai_ship(struct World* world, Vector3 position, const char* type)
```
Creates an AI-controlled ship with specified behavior type.

#### `EntityID create_sun(struct World* world, Vector3 position)`
```c
EntityID create_sun(struct World* world, Vector3 position)
```
Creates a large celestial body for scene decoration.

---

## Constants and Enums

### Component Types
```c
typedef enum {
    COMPONENT_TRANSFORM  = 1 << 0,
    COMPONENT_PHYSICS    = 1 << 1,
    COMPONENT_COLLISION  = 1 << 2,
    COMPONENT_AI         = 1 << 3,
    COMPONENT_RENDERABLE = 1 << 4,
    COMPONENT_PLAYER     = 1 << 5,
} ComponentType;
```

### AI States
```c
typedef enum {
    AI_STATE_IDLE,
    AI_STATE_PATROLLING,
    AI_STATE_CHASING,
    AI_STATE_FLEEING,
    AI_STATE_ATTACKING
} AIState;
```

### System Types
```c
typedef enum {
    SYSTEM_PHYSICS,
    SYSTEM_COLLISION,
    SYSTEM_AI,
    SYSTEM_RENDER,
    SYSTEM_COUNT
} SystemType;
```

---

## Usage Examples

### Basic Entity Creation
```c
// Create world
struct World world;
world_init(&world);

// Create player entity
EntityID player = entity_create(&world);
entity_add_component(&world, player, COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_PLAYER);

// Configure components
struct Transform* transform = entity_get_transform(&world, player);
transform->position = (Vector3){0.0f, 0.0f, 0.0f};

struct Physics* physics = entity_get_physics(&world, player);
physics->mass = 80.0f;
physics->drag = 0.98f;
```

### System Update Loop
```c
struct SystemScheduler scheduler;
system_scheduler_init(&scheduler);

// Game loop
while (running) {
    float delta_time = calculate_delta_time();
    
    world_update(&world, delta_time);
    system_scheduler_update(&scheduler, &world, delta_time);
    
    render_3d_frame(&world, &render_config);
}
```

### Entity Iteration Pattern
```c
// Process all entities with physics
for (uint32_t i = 0; i < world.entity_count; i++) {
    struct Entity* entity = &world.entities[i];
    if (!(entity->component_mask & COMPONENT_PHYSICS)) continue;
    
    struct Physics* physics = entity->physics;
    struct Transform* transform = entity->transform;
    
    // Update physics...
}
```

#### `Entity* entity_create(EntityType type, Vector3D position)`
Creates a new entity with default properties.

#### `void entity_update(Entity* entity, Universe* universe, float deltaTime)`
Updates an entity's physics, AI, and behavior.

### AI Integration

#### `void entity_request_ai_task(Entity* entity, AIEngine* engine, AITaskType task_type)`
Requests AI processing for the entity.

#### `void entity_process_ai_response(Entity* entity, const char* response, AITaskType task_type)`
Processes an AI response and updates the entity's behavior.

### Level-of-Detail System

#### `void entity_update_lod(Entity* entity, Vector3D player_position)`
Updates an entity's Level-of-Detail based on its distance to the player, adjusting AI update frequency.

---

## AI Engine API

The AI Engine provides neural language model capabilities for all entities.

### Core Functions

#### `bool ai_engine_init(AIEngine* engine, const char* model_path)`
Initializes the AI engine with a neural language model.

#### `void ai_engine_process_tasks(AIEngine* engine, float deltaTime)`
Processes queued AI tasks with priority scheduling.

### Task Submission

#### `void ai_request_dialog(AIEngine* engine, Entity* entity, const char* player_message, void (*response_callback)(Entity*, char*))`
Requests high-priority dialog generation.

#### `void ai_request_behavior_update(AIEngine* engine, Entity* entity)`
Requests a behavior script generation for an entity.

---

## Dialog System API

The Dialog System provides real-time conversations with streaming responses.

### Core Functions

#### `bool dialog_start_conversation(DialogSession* dialog, Entity* entity, const char* player_message)`
Initiates a conversation with an AI entity.

#### `void dialog_update(DialogSession* dialog, float deltaTime)`
Updates the dialog session, displaying characters with a typewriter effect.

---

## Error Handling API

The Error Handling system provides comprehensive error tracking and recovery.

### Logging Macros

- `LOG_INFO(category, format, ...)`
- `LOG_WARNING(category, code, format, ...)`
- `LOG_ERROR(category, code, format, ...)`
- `LOG_CRITICAL(category, code, format, ...)`

### Validation Macros

- `VALIDATE_PARAM(param, error_code)`
- `SAFE_MALLOC(ptr, size, error_code)`

---

## Llama Integration API

The Llama Integration provides C bindings for neural language model inference.

### Core Functions

#### `bool llama_init(LlamaInference* inference, const char* model_path, int max_context_size)`
Initializes the Llama inference engine.

#### `char* llama_generate(LlamaInference* inference, const char* prompt, int max_tokens)`
Generates a text response from a prompt.

#### `bool llama_generate_stream(LlamaInference* inference, const char* prompt, int max_tokens, LlamaStreamCallback callback, void* user_data)`
Generates a streaming text response for real-time display.
