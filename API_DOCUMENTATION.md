# API Documentation - Neural AI-Driven 3D Space Game

## Overview

This document provides comprehensive API documentation for the neural AI-driven 3D space game. The architecture is built around three core layers: Game, Universe, and AI, each providing specific functionality for creating a living, intelligent game world.

## Table of Contents

1. [Universe API](#universe-api)
2. [Entity API](#entity-api)
3. [AI Engine API](#ai-engine-api)
4. [Dialog System API](#dialog-system-api)
5. [Error Handling API](#error-handling-api)
6. [Llama Integration API](#llama-integration-api)

---

## Universe API

The Universe module manages the entire game world, including all entities, AI processing, and spatial relationships.

### Core Functions

#### `Universe* universe_create(const char* ai_model_path)`
**Purpose**: Creates and initializes a new universe with neural AI capabilities.

**Parameters**:
- `ai_model_path`: Path to the AI model file (e.g., "models/gemma-2b-it.gguf")

**Returns**: Pointer to Universe structure, or NULL on failure

**Example**:
```c
Universe* universe = universe_create("models/gemma-2b-it.gguf");
if (!universe) {
    LOG_ERROR(ERROR_CATEGORY_AI, ERROR_INITIALIZATION_FAILED, "Failed to create universe");
    return false;
}
```

#### `void universe_destroy(Universe* universe)`
**Purpose**: Safely destroys a universe and all its entities.

**Parameters**:
- `universe`: Pointer to Universe to destroy

**Side Effects**: 
- Destroys all entities in the universe
- Shuts down AI engine
- Frees all allocated memory

#### `void universe_update(Universe* universe, float deltaTime)`
**Purpose**: Updates the universe simulation for one frame.

**Parameters**:
- `universe`: Pointer to Universe to update
- `deltaTime`: Time elapsed since last update (in seconds)

**Operations**:
- Updates all entity physics and AI
- Processes AI task queues
- Updates Level-of-Detail system
- Handles inter-entity communication

### Entity Management

#### `void universe_add_entity(Universe* universe, Entity* entity)`
**Purpose**: Adds an entity to the universe.

**Parameters**:
- `universe`: Target universe
- `entity`: Entity to add

**Side Effects**:
- Assigns unique ID to entity
- Adds to spatial partitioning system
- Triggers `on_entity_created` event

#### `Entity* universe_spawn_entity(Universe* universe, EntityType type, Vector3D position, const char* ai_personality)`
**Purpose**: Creates and spawns a new AI entity with specified personality.

**Parameters**:
- `universe`: Target universe
- `type`: Type of entity (ENTITY_AI_FIGHTER, ENTITY_AI_TRADER, etc.)
- `position`: 3D position for entity
- `ai_personality`: Personality prompt for AI behavior

**Returns**: Pointer to created entity, or NULL on failure

**Example**:
```c
Entity* trader = universe_spawn_entity(universe, ENTITY_AI_TRADER,
    (Vector3D){100.0f, 0.0f, 50.0f},
    "Friendly merchant who loves to chat about trade routes");
if (trader) {
    trader->name = strdup("Captain Zorax");
}
```

### Spatial Queries

#### `int universe_find_entities_near(Universe* universe, Vector3D position, float radius, Entity** results, int max_results)`
**Purpose**: Finds all entities within a specified radius of a position.

**Parameters**:
- `universe`: Universe to search
- `position`: Center point for search
- `radius`: Search radius
- `results`: Array to store found entities
- `max_results`: Maximum number of entities to return

**Returns**: Number of entities found

**Usage**: Used by AI system to build spatial context for entities.

---

## Entity API

The Entity system provides autonomous AI agents with personality, sensors, and behavior.

### Core Functions

#### `Entity* entity_create(EntityType type, Vector3D position)`
**Purpose**: Creates a new entity with default properties.

**Parameters**:
- `type`: Entity type (determines default behavior and capabilities)
- `position`: Initial 3D position

**Returns**: Pointer to created entity, or NULL on failure

#### `void entity_update(Entity* entity, Universe* universe, float deltaTime)`
**Purpose**: Updates entity physics, AI, and behavior.

**Parameters**:
- `entity`: Entity to update
- `universe`: Universe context for spatial queries
- `deltaTime`: Time elapsed since last update

**Operations**:
- Updates position based on velocity
- Processes AI responses
- Handles communication cooldowns
- Calls custom update callbacks

### AI Integration

#### `void entity_request_ai_task(Entity* entity, AIEngine* engine, AITaskType task_type)`
**Purpose**: Requests AI processing for the entity.

**Parameters**:
- `entity`: Entity requesting AI processing
- `engine`: AI engine to process the request
- `task_type`: Type of AI task (DIALOG, BEHAVIOR_SCRIPT, NAVIGATION, etc.)

**Example**:
```c
// Request behavior update for idle entity
entity_request_ai_task(entity, universe->ai_engine, AI_TASK_BEHAVIOR_SCRIPT);
```

#### `void entity_process_ai_response(Entity* entity, const char* response, AITaskType task_type)`
**Purpose**: Processes AI response and updates entity behavior.

**Parameters**:
- `entity`: Entity receiving AI response
- `response`: Generated AI response text
- `task_type`: Type of task that generated the response

**Operations**:
- Parses AI response for commands
- Updates entity state based on response
- Calls custom AI response handlers

### Level-of-Detail System

#### `void entity_update_lod(Entity* entity, Vector3D player_position)`
**Purpose**: Updates entity's Level-of-Detail based on distance to player.

**Parameters**:
- `entity`: Entity to update
- `player_position`: Current player position

**Effects**:
- Adjusts AI update frequency based on distance
- Optimizes performance for distant entities
- Maintains full AI processing for nearby entities

**Distance Bands**:
- 0-100m: High frequency AI updates (0.2-0.5s)
- 100-500m: Medium frequency updates (1s)
- 500-2000m: Low frequency updates (2s)
- 2000m+: Very low frequency updates (10s)

---

## AI Engine API

The AI Engine provides neural language model capabilities for all entities.

### Core Functions

#### `bool ai_engine_init(AIEngine* engine, const char* model_path)`
**Purpose**: Initializes the AI engine with a neural language model.

**Parameters**:
- `engine`: AI engine structure to initialize
- `model_path`: Path to neural model file

**Returns**: true on success, false on failure

**Operations**:
- Loads neural language model
- Initializes task queues
- Sets up performance monitoring

#### `void ai_engine_process_tasks(AIEngine* engine, float deltaTime)`
**Purpose**: Processes queued AI tasks with priority scheduling.

**Parameters**:
- `engine`: AI engine to process
- `deltaTime`: Time available for processing

**Task Processing Order**:
1. High Priority (Dialog): <200ms response time
2. Medium Priority (Combat/Navigation): ~500ms response time
3. Low Priority (Background): 2-10s response time

### Task Submission

#### `void ai_request_dialog(AIEngine* engine, Entity* entity, const char* player_message, void (*response_callback)(Entity*, char*))`
**Purpose**: Requests high-priority dialog generation.

**Parameters**:
- `engine`: AI engine to process request
- `entity`: Entity participating in dialog
- `player_message`: Message from player
- `response_callback`: Function to call with AI response

**Priority**: Highest (processed within 200ms)

#### `void ai_request_behavior_update(AIEngine* engine, Entity* entity)`
**Purpose**: Requests behavior script generation for entity.

**Parameters**:
- `engine`: AI engine to process request
- `entity`: Entity needing behavior update

**Priority**: Low (processed within 2-10 seconds)

### Context Building

#### `AIContext* ai_build_context(Entity* entity, Universe* universe)`
**Purpose**: Builds rich contextual information for AI processing.

**Parameters**:
- `entity`: Entity to build context for
- `universe`: Universe for spatial queries

**Returns**: Pointer to AIContext structure

**Context Includes**:
- Spatial awareness (position, velocity, nearby entities)
- Personality traits and speaking style
- Conversation history and relationships
- Current goals and motivations
- Sensor data and environmental conditions

---

## Dialog System API

The Dialog System provides real-time conversations with streaming responses.

### Core Functions

#### `bool dialog_start_conversation(DialogSession* dialog, Entity* entity, const char* player_message)`
**Purpose**: Initiates a conversation with an AI entity.

**Parameters**:
- `dialog`: Dialog session structure
- `entity`: AI entity to converse with
- `player_message`: Initial message from player

**Returns**: true if conversation started successfully

**Example**:
```c
DialogSession dialog;
if (dialog_start_conversation(&dialog, trader_entity, "Hello there!")) {
    // Conversation started, AI will begin generating response
}
```

#### `void dialog_update(DialogSession* dialog, float deltaTime)`
**Purpose**: Updates dialog session with typewriter effect.

**Parameters**:
- `dialog`: Dialog session to update
- `deltaTime`: Time elapsed since last update

**Effects**:
- Displays characters at typewriter speed (20 chars/second)
- Handles streaming response completion
- Updates conversation history

---

## Error Handling API

The Error Handling system provides comprehensive error tracking and recovery.

### Core Functions

#### `bool error_system_init(ErrorConfig* config)`
**Purpose**: Initializes the error handling system.

**Parameters**:
- `config`: Error handling configuration

**Example**:
```c
ErrorConfig config = {
    .log_to_console = true,
    .log_to_file = false,
    .abort_on_critical = true,
    .min_log_level = ERROR_LEVEL_INFO
};
error_system_init(&config);
```

### Logging Macros

#### `LOG_INFO(category, format, ...)`
**Purpose**: Logs informational messages.

#### `LOG_WARNING(category, code, format, ...)`
**Purpose**: Logs warning messages with error code.

#### `LOG_ERROR(category, code, format, ...)`
**Purpose**: Logs error messages with error code.

#### `LOG_CRITICAL(category, code, format, ...)`
**Purpose**: Logs critical errors (may abort program).

### Validation Macros

#### `VALIDATE_PARAM(param, error_code)`
**Purpose**: Validates parameter and returns false on failure.

#### `SAFE_MALLOC(ptr, size, error_code)`
**Purpose**: Allocates memory with error checking.

**Example**:
```c
bool create_entity(Entity** entity, EntityType type) {
    VALIDATE_PARAM(entity, ERROR_NULL_POINTER);
    SAFE_MALLOC(*entity, sizeof(Entity), ERROR_OUT_OF_MEMORY);
    return true;
}
```

---

## Llama Integration API

The Llama Integration provides C bindings for neural language model inference.

### Core Functions

#### `bool llama_init(LlamaInference* inference, const char* model_path, int max_context_size)`
**Purpose**: Initializes Llama inference engine.

**Parameters**:
- `inference`: Inference structure to initialize
- `model_path`: Path to GGML model file
- `max_context_size`: Maximum context tokens

#### `char* llama_generate(LlamaInference* inference, const char* prompt, int max_tokens)`
**Purpose**: Generates text response from prompt.

**Parameters**:
- `inference`: Initialized inference engine
- `prompt`: Input prompt for generation
- `max_tokens`: Maximum tokens to generate

**Returns**: Generated text (must be freed by caller)

#### `bool llama_generate_stream(LlamaInference* inference, const char* prompt, int max_tokens, LlamaStreamCallback callback, void* user_data)`
**Purpose**: Generates streaming text response for real-time display.

**Parameters**:
- `inference`: Initialized inference engine
- `prompt`: Input prompt for generation
- `max_tokens`: Maximum tokens to generate
- `callback`: Function called for each generated token
- `user_data`: User data passed to callback

**Usage**: Used for real-time dialog with typewriter effect.

---

## Best Practices

### Error Handling
- Always validate parameters using `VALIDATE_PARAM` macros
- Use `SAFE_MALLOC` for memory allocation
- Log errors with appropriate severity levels
- Check return values of all API functions

### AI Integration
- Build rich context for AI requests using spatial and social information
- Use appropriate task types for different AI operations
- Implement LOD system to optimize AI processing for distant entities
- Cache AI context for performance optimization

### Memory Management
- Free all allocated strings and structures
- Use proper cleanup functions (`universe_destroy`, `entity_destroy`)
- Check for memory leaks during development

### Performance
- Use spatial partitioning for efficient neighbor queries
- Implement Level-of-Detail system for AI processing
- Monitor AI response times and adjust priorities as needed
- Limit concurrent AI tasks based on available computational resources

---

This API documentation provides the foundation for building neural AI-driven game experiences with natural language capabilities, autonomous entity behavior, and real-time performance optimization.
