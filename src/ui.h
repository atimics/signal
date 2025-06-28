#pragma once

#include "core.h"
#include <SDL.h>
#include <stdbool.h>

// ============================================================================
// UI TYPES AND STRUCTURES
// ============================================================================

typedef struct {
    int x, y, w, h;
} UIRect;

typedef struct {
    float x, y;  // Position in world coordinates
    float size;  // Size/zoom level
    bool is_target;
    char label[32];
} RadarBlip;

typedef struct {
    RadarBlip blips[64];
    int blip_count;
    Vector3 center_position;  // Player position
    float radar_range;        // Max distance to show
    float scale;             // Pixels per world unit
} RadarDisplay;

typedef struct {
    char sender[32];
    char message[128];
    float timestamp;
    bool is_player;
} CommMessage;

typedef struct {
    CommMessage messages[16];
    int message_count;
    int scroll_offset;
    bool show_input;
    char input_buffer[128];
} CommPanel;

typedef struct {
    float speed;
    float heading;
    float health;
    float shields;
    float fuel;
    char target_name[32];
    float target_distance;
    bool target_hostile;
} HUDData;

typedef struct {
    // Screen layout
    UIRect cockpit_frame;
    UIRect radar_area;
    UIRect comm_area;
    UIRect hud_area;
    UIRect main_view;
    
    // UI state
    RadarDisplay radar;
    CommPanel comm;
    HUDData hud;
    
    // Rendering
    SDL_Renderer* renderer;
    bool initialized;
} CockpitUI;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

// Initialization and cleanup
bool cockpit_ui_init(CockpitUI* ui, SDL_Renderer* renderer, int screen_width, int screen_height);
void cockpit_ui_cleanup(CockpitUI* ui);

// Update functions
void cockpit_ui_update(CockpitUI* ui, struct World* world, EntityID player_id, float delta_time);
void cockpit_ui_render(CockpitUI* ui);

// Message system
void cockpit_ui_add_message(CockpitUI* ui, const char* sender, const char* message, bool is_player);

// Individual component functions
void radar_update(RadarDisplay* radar, struct World* world, EntityID player_id);
void radar_render(const RadarDisplay* radar, SDL_Renderer* renderer, const UIRect* area);

void comm_add_message(CommPanel* comm, const char* sender, const char* message, bool is_player);
void comm_render(const CommPanel* comm, SDL_Renderer* renderer, const UIRect* area);

void hud_update(HUDData* hud, struct World* world, EntityID player_id);
void hud_render(const HUDData* hud, SDL_Renderer* renderer, const UIRect* area);

// Utility functions
void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, uint8_t r, uint8_t g, uint8_t b);
void draw_filled_rect(SDL_Renderer* renderer, const UIRect* rect, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void draw_outline_rect(SDL_Renderer* renderer, const UIRect* rect, uint8_t r, uint8_t g, uint8_t b);
