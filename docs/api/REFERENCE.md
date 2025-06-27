# API Reference

This document provides API documentation for the CGame engine.

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
Creates and initializes a new universe with neural AI capabilities.

#### `void universe_destroy(Universe* universe)`
Safely destroys a universe and all its entities.

#### `void universe_update(Universe* universe, float deltaTime)`
Updates the universe simulation for one frame.

### Entity Management

#### `void universe_add_entity(Universe* universe, Entity* entity)`
Adds an entity to the universe.

#### `Entity* universe_spawn_entity(Universe* universe, EntityType type, Vector3D position, const char* ai_personality)`
Creates and spawns a new AI entity with a specified personality.

### Spatial Queries

#### `int universe_find_entities_near(Universe* universe, Vector3D position, float radius, Entity** results, int max_results)`
Finds all entities within a specified radius of a position.

---

## Entity API

The Entity system provides autonomous AI agents with personality, sensors, and behavior.

### Core Functions

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
