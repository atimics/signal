#include "cgame.h"
#include <stdio.h>
#include <time.h>

// Screen dimensions
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 800;

bool init_game(struct Game* game) {
    VALIDATE_PARAM(game, ERROR_NULL_POINTER);
    
    LOG_INFO(ERROR_CATEGORY_GENERAL, "Initializing game systems...");
    
    // Initialize error handling system first
    struct ErrorConfig error_config = {
        .log_to_console = true,
        .log_to_file = false,
        .abort_on_critical = true,
        .min_log_level = ERROR_LEVEL_INFO,
        .log_file_path = NULL,
        .log_file = NULL
    };
    
    if (!error_system_init(&error_config)) {
        printf("❌ Failed to initialize error handling system\n");
        return false;
    }
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        LOG_ERROR(ERROR_CATEGORY_GRAPHICS, ERROR_SDL_ERROR, 
                 "SDL could not initialize: %s", SDL_GetError());
        return false;
    }
    LOG_INFO(ERROR_CATEGORY_GRAPHICS, "SDL initialized successfully");

    // Create window
    game->window = SDL_CreateWindow("Neural AI-Driven 3D Space Universe", 
                                   SDL_WINDOWPOS_UNDEFINED, 
                                   SDL_WINDOWPOS_UNDEFINED, 
                                   SCREEN_WIDTH, SCREEN_HEIGHT, 
                                   SDL_WINDOW_SHOWN);
    if (game->window == NULL) {
        LOG_ERROR(ERROR_CATEGORY_GRAPHICS, ERROR_SDL_ERROR,
                 "Window could not be created: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }
    LOG_INFO(ERROR_CATEGORY_GRAPHICS, "Game window created (%dx%d)", SCREEN_WIDTH, SCREEN_HEIGHT);

    // Create renderer
    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED);
    if (game->renderer == NULL) {
        LOG_ERROR(ERROR_CATEGORY_GRAPHICS, ERROR_SDL_ERROR,
                 "Renderer could not be created: %s", SDL_GetError());
        SDL_DestroyWindow(game->window);
        SDL_Quit();
        return false;
    }
    LOG_INFO(ERROR_CATEGORY_GRAPHICS, "Hardware-accelerated renderer created");

    // Initialize game state
    game->running = true;
    game->time = 0.0f;
    game->mouse_captured = false;
    game->last_mouse_x = SCREEN_WIDTH / 2;
    game->last_mouse_y = SCREEN_HEIGHT / 2;
    
    // Clear keys array
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        game->keys[i] = false;
    }

    // Initialize game components
    LOG_INFO(ERROR_CATEGORY_GENERAL, "Initializing game components...");
    init_meshes();
    init_spaceship(&game->player);
    init_camera(&game->camera);
    init_stars(game);
    init_particles(game);
    
    // Initialize AI-driven universe
    LOG_INFO(ERROR_CATEGORY_AI, "Initializing neural AI-driven universe...");
    game->universe = universe_create("models/gemma-2b-it.gguf"); // Mock model path
    if (!game->universe) {
        LOG_ERROR(ERROR_CATEGORY_AI, ERROR_INITIALIZATION_FAILED, 
                 "Failed to create AI universe");
        cleanup_game(game);
        return false;
    }
    LOG_INFO(ERROR_CATEGORY_AI, "AI universe created successfully");
    
    // Initialize dialog system
    SAFE_MALLOC(game->active_dialog, sizeof(struct DialogSession), ERROR_OUT_OF_MEMORY);
    memset(game->active_dialog, 0, sizeof(struct DialogSession));
    LOG_INFO(ERROR_CATEGORY_AI, "Dialog system initialized");
    
    // Create demo AI entities for testing
    game->demo_entity_count = 0;
    LOG_INFO(ERROR_CATEGORY_AI, "Creating demo AI entities...");
    
    // Create a friendly trader entity
    struct Entity* trader = universe_spawn_entity(game->universe, ENTITY_AI_TRADER, 
                                         (Vector3D){100.0f, 0.0f, 50.0f}, 
                                         "Friendly merchant who loves to chat about trade routes and cosmic gossip");
    if (trader) {
        trader->name = strdup("Captain Zorax");
        game->demo_entities[game->demo_entity_count++] = trader;
        LOG_INFO(ERROR_CATEGORY_AI, "Created AI trader: %s", trader->name);
    } else {
        LOG_WARNING(ERROR_CATEGORY_AI, ERROR_INITIALIZATION_FAILED,
                   "Failed to create trader entity");
    }
    
    // Create a patrol fighter
    struct Entity* fighter = universe_spawn_entity(game->universe, ENTITY_AI_FIGHTER,
                                          (Vector3D){-80.0f, 30.0f, -100.0f},
                                          "Professional military pilot on patrol duty, suspicious of unknown vessels");
    if (fighter) {
        fighter->name = strdup("Lieutenant Vex");
        game->demo_entities[game->demo_entity_count++] = fighter;
    }
    
    // Create an explorer
    struct Entity* explorer = universe_spawn_entity(game->universe, ENTITY_AI_EXPLORER,
                                           (Vector3D){0.0f, -200.0f, 150.0f},
                                           "Curious explorer fascinated by cosmic phenomena and ancient ruins");
    if (explorer) {
        explorer->name = strdup("Dr. Kira Nova");
        game->demo_entities[game->demo_entity_count++] = explorer;
    }
    
    printf("✅ Created %d demo AI entities\n", game->demo_entity_count);
    
    // Initialize random seed
    srand((unsigned int)time(NULL));

    return true;
}

void cleanup_game(struct Game* game) {
    if (!game) return;
    
    LOG_INFO(ERROR_CATEGORY_GENERAL, "Shutting down game systems...");
    
    // Cleanup AI universe
    if (game->universe) {
        LOG_INFO(ERROR_CATEGORY_AI, "Destroying AI universe...");
        universe_destroy(game->universe);
        game->universe = NULL;
    }
    
    // Cleanup dialog system
    if (game->active_dialog) {
        LOG_INFO(ERROR_CATEGORY_AI, "Ending dialog session...");
        dialog_end_conversation(game->active_dialog);
        free(game->active_dialog);
        game->active_dialog = NULL;
    }
    
    // Cleanup SDL resources
    if (game->renderer) {
        LOG_INFO(ERROR_CATEGORY_GRAPHICS, "Destroying renderer...");
        SDL_DestroyRenderer(game->renderer);
        game->renderer = NULL;
    }
    
    if (game->window) {
        LOG_INFO(ERROR_CATEGORY_GRAPHICS, "Destroying window...");
        SDL_DestroyWindow(game->window);
        game->window = NULL;
    }
    
    LOG_INFO(ERROR_CATEGORY_GRAPHICS, "Quitting SDL...");
    SDL_Quit();
    
    // Shutdown error handling system last
    LOG_INFO(ERROR_CATEGORY_GENERAL, "Game cleanup complete");
    error_system_shutdown();
}

void update_game(struct Game* game) {
    float deltaTime = 1.0f / 60.0f; // Assuming 60 FPS
    game->time += deltaTime;
    
    // Update spaceship physics
    update_spaceship(&game->player, deltaTime);
    
    // Update camera to follow spaceship
    update_camera(&game->camera, &game->player);
    
    // Update AI universe (this is where the magic happens!)
    if (game->universe) {
        // Update player position in universe for LOD calculations
        game->universe->player_position = (Vector3D){
            game->player.position.x,
            game->player.position.y, 
            game->player.position.z
        };
        
        // Process AI tasks and update entities
        universe_update(game->universe, deltaTime);
        
        // Update dialog system
        if (game->active_dialog && game->active_dialog->is_speaking) {
            dialog_update(game->active_dialog, deltaTime);
        }
        
        // Demo: Occasionally trigger AI conversations
        static float ai_demo_timer = 0.0f;
        ai_demo_timer += deltaTime;
        
        if (ai_demo_timer > 15.0f && !game->active_dialog->is_speaking) {
            // Start random conversation with nearby entity
            for (int i = 0; i < game->demo_entity_count; i++) {
                struct Entity* entity = game->demo_entities[i];
                if (entity && entity->distance_to_player < 200.0f) {
                    printf("🎯 Starting demo conversation with %s\n", entity->name);
                    dialog_start_conversation(game->active_dialog, entity, "Hello there!");
                    ai_demo_timer = 0.0f;
                    break;
                }
            }
        }
    }
    
    // Add engine particles based on throttle
    if (fabs(game->player.throttle) > 0.1f) {
        int particle_rate = (int)(fabs(game->player.throttle) * 10) + 1;
        if ((int)(game->time * 60) % (6 - particle_rate) == 0) {
            add_engine_particle(game);
        }
    }
    
    // Add afterburner particles
    if (game->player.afterburner) {
        if ((int)(game->time * 60) % 2 == 0) {
            add_engine_particle(game);
            add_engine_particle(game); // Extra particles for afterburner
        }
    }
    
    // Update particles
    update_particles_system(game, deltaTime);
    
    // Update starfield
    update_stars(game, deltaTime);
}
