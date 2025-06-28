#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// INITIALIZATION AND CLEANUP
// ============================================================================

bool cockpit_ui_init(CockpitUI* ui, SDL_Renderer* renderer, int screen_width, int screen_height) {
    if (!ui || !renderer) return false;
    
    memset(ui, 0, sizeof(CockpitUI));
    ui->renderer = renderer;
    
    // Define cockpit layout - taking up border areas, leaving center for 3D view
    int border = 80;
    int radar_size = 200;
    int comm_width = 300;
    int hud_height = 60;
    
    // Main 3D view (center, reduced by UI borders)
    ui->main_view = (UIRect){
        border, border,
        screen_width - 2 * border,
        screen_height - 2 * border - hud_height
    };
    
    // Radar (top-left corner)
    ui->radar_area = (UIRect){
        10, 10,
        radar_size, radar_size
    };
    
    // Communications panel (top-right)
    ui->comm_area = (UIRect){
        screen_width - comm_width - 10, 10,
        comm_width, 250
    };
    
    // HUD (bottom strip)
    ui->hud_area = (UIRect){
        0, screen_height - hud_height,
        screen_width, hud_height
    };
    
    // Cockpit frame (entire screen border)
    ui->cockpit_frame = (UIRect){0, 0, screen_width, screen_height};
    
    // Initialize radar
    ui->radar.radar_range = 100.0f;
    ui->radar.scale = (float)radar_size / (ui->radar.radar_range * 2.0f);
    
    // Add initial comm messages
    comm_add_message(&ui->comm, "TOWER", "Welcome to Nexus Spaceport", false);
    comm_add_message(&ui->comm, "CONTROL", "Landing Pad Alpha cleared", false);
    comm_add_message(&ui->comm, "SYSTEM", "Cockpit systems online", false);
    comm_add_message(&ui->comm, "NAV", "GPS locked - surface coords", false);
    comm_add_message(&ui->comm, "COMM", "Planetary comm net active", false);
    comm_add_message(&ui->comm, "TRAFFIC", "Multiple ships on approach", false);
    
    ui->initialized = true;
    printf("🎛️ Cockpit UI initialized (%dx%d)\n", screen_width, screen_height);
    return true;
}

void cockpit_ui_cleanup(CockpitUI* ui) {
    if (!ui) return;
    ui->initialized = false;
    printf("🎛️ Cockpit UI cleaned up\n");
}

// ============================================================================
// MAIN UPDATE AND RENDER
// ============================================================================

void cockpit_ui_update(CockpitUI* ui, struct World* world, EntityID player_id, float delta_time) {
    if (!ui || !ui->initialized || !world) return;
    
    // Update radar
    radar_update(&ui->radar, world, player_id);
    
    // Update HUD data
    hud_update(&ui->hud, world, player_id);
}

void cockpit_ui_render(CockpitUI* ui) {
    if (!ui || !ui->initialized || !ui->renderer) return;
    
    // Draw cockpit frame (decorative border)
    SDL_SetRenderDrawColor(ui->renderer, 40, 60, 80, 255);
    
    // Top border
    SDL_Rect top_border = {0, 0, ui->cockpit_frame.w, 80};
    SDL_RenderFillRect(ui->renderer, &top_border);
    
    // Bottom border  
    SDL_Rect bottom_border = {0, ui->cockpit_frame.h - 80, ui->cockpit_frame.w, 80};
    SDL_RenderFillRect(ui->renderer, &bottom_border);
    
    // Left border
    SDL_Rect left_border = {0, 80, 80, ui->cockpit_frame.h - 160};
    SDL_RenderFillRect(ui->renderer, &left_border);
    
    // Right border
    SDL_Rect right_border = {ui->cockpit_frame.w - 80, 80, 80, ui->cockpit_frame.h - 160};
    SDL_RenderFillRect(ui->renderer, &right_border);
    
    // Render individual components
    radar_render(&ui->radar, ui->renderer, &ui->radar_area);
    comm_render(&ui->comm, ui->renderer, &ui->comm_area);
    hud_render(&ui->hud, ui->renderer, &ui->hud_area);
}

// ============================================================================
// RADAR SYSTEM
// ============================================================================

void radar_update(RadarDisplay* radar, struct World* world, EntityID player_id) {
    if (!radar || !world) return;
    
    radar->blip_count = 0;
    
    // Get player position for radar center
    struct Transform* player_transform = entity_get_transform(world, player_id);
    if (player_transform) {
        radar->center_position = player_transform->position;
    }
    
    // Scan all entities and add radar blips
    for (uint32_t i = 0; i < world->entity_count && radar->blip_count < 64; i++) {
        struct Entity* entity = &world->entities[i];
        EntityID entity_id = i + 1;
        
        if (entity_id == player_id) continue;  // Don't show player on radar
        
        if (entity->component_mask & COMPONENT_TRANSFORM) {
            struct Transform* transform = entity_get_transform(world, entity_id);
            if (!transform) continue;
            
            // Calculate distance from player
            float dx = transform->position.x - radar->center_position.x;
            float dz = transform->position.z - radar->center_position.z;
            float distance = sqrtf(dx * dx + dz * dz);
            
            // Only show objects within radar range
            if (distance <= radar->radar_range) {
                RadarBlip* blip = &radar->blips[radar->blip_count++];
                
                // Convert world position to radar screen position
                blip->x = dx * radar->scale;
                blip->y = dz * radar->scale;
                blip->size = 2.0f;
                blip->is_target = false;
                
                // Determine blip type and label
                if (entity->component_mask & COMPONENT_AI) {
                    strcpy(blip->label, "AI");
                } else if (entity->component_mask & COMPONENT_COLLISION) {
                    struct Collision* collision = entity_get_collision(world, entity_id);
                    if (collision && collision->radius > 15.0f) {
                        strcpy(blip->label, "SUN");
                        blip->size = 8.0f;
                    } else if (collision && collision->radius > 5.0f) {
                        strcpy(blip->label, "PLT");
                        blip->size = 4.0f;
                    } else {
                        strcpy(blip->label, "AST");
                        blip->size = 2.0f;
                    }
                }
            }
        }
    }
}

void radar_render(const RadarDisplay* radar, SDL_Renderer* renderer, const UIRect* area) {
    if (!radar || !renderer || !area) return;
    
    // Draw radar background
    draw_filled_rect(renderer, area, 0, 20, 0, 180);
    draw_outline_rect(renderer, area, 0, 255, 0);
    
    // Draw radar grid
    int center_x = area->x + area->w / 2;
    int center_y = area->y + area->h / 2;
    
    SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
    
    // Crosshairs
    SDL_RenderDrawLine(renderer, center_x - 20, center_y, center_x + 20, center_y);
    SDL_RenderDrawLine(renderer, center_x, center_y - 20, center_x, center_y + 20);
    
    // Range circles
    for (int i = 1; i <= 3; i++) {
        int radius = i * area->w / 6;
        for (int angle = 0; angle < 360; angle += 5) {
            float rad = angle * M_PI / 180.0f;
            int x1 = center_x + (int)(radius * cosf(rad));
            int y1 = center_y + (int)(radius * sinf(rad));
            int x2 = center_x + (int)((radius + 1) * cosf(rad));
            int y2 = center_y + (int)((radius + 1) * sinf(rad));
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }
    
    // Draw blips
    for (int i = 0; i < radar->blip_count; i++) {
        const RadarBlip* blip = &radar->blips[i];
        
        int blip_x = center_x + (int)blip->x;
        int blip_y = center_y + (int)blip->y;
        
        // Clamp to radar area
        if (blip_x < area->x || blip_x >= area->x + area->w ||
            blip_y < area->y || blip_y >= area->y + area->h) {
            continue;
        }
        
        // Choose color based on blip type
        if (strcmp(blip->label, "SUN") == 0) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);  // Yellow for sun
        } else if (strcmp(blip->label, "PLT") == 0) {
            SDL_SetRenderDrawColor(renderer, 0, 100, 255, 255);  // Blue for planets
        } else if (strcmp(blip->label, "AI") == 0) {
            SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);  // Red for AI ships
        } else {
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);  // Gray for asteroids
        }
        
        // Draw blip as a small filled rectangle
        SDL_Rect blip_rect = {
            blip_x - (int)blip->size/2, 
            blip_y - (int)blip->size/2,
            (int)blip->size, 
            (int)blip->size
        };
        SDL_RenderFillRect(renderer, &blip_rect);
    }
    
    // Draw radar label
    draw_text(renderer, "RADAR", area->x + 5, area->y + area->h - 15, 0, 255, 0);
}

// ============================================================================
// COMMUNICATIONS PANEL
// ============================================================================

void comm_add_message(CommPanel* comm, const char* sender, const char* message, bool is_player) {
    if (!comm || !sender || !message) return;
    
    // Shift messages up if at limit
    if (comm->message_count >= 16) {
        for (int i = 0; i < 15; i++) {
            comm->messages[i] = comm->messages[i + 1];
        }
        comm->message_count = 15;
    }
    
    // Add new message
    CommMessage* msg = &comm->messages[comm->message_count++];
    strncpy(msg->sender, sender, sizeof(msg->sender) - 1);
    msg->sender[sizeof(msg->sender) - 1] = '\0';
    strncpy(msg->message, message, sizeof(msg->message) - 1);
    msg->message[sizeof(msg->message) - 1] = '\0';
    msg->is_player = is_player;
    msg->timestamp = 0;  // Would use actual time in real implementation
}

void comm_render(const CommPanel* comm, SDL_Renderer* renderer, const UIRect* area) {
    if (!comm || !renderer || !area) return;
    
    // Draw comm background
    draw_filled_rect(renderer, area, 0, 0, 20, 180);
    draw_outline_rect(renderer, area, 0, 200, 255);
    
    // Draw title
    draw_text(renderer, "COMMUNICATIONS", area->x + 5, area->y + 5, 0, 200, 255);
    
    // Draw messages
    int y_offset = 25;
    for (int i = 0; i < comm->message_count && y_offset < area->h - 20; i++) {
        const CommMessage* msg = &comm->messages[i];
        
        // Color based on sender type
        uint8_t r = msg->is_player ? 100 : 200;
        uint8_t g = msg->is_player ? 255 : 200;
        uint8_t b = msg->is_player ? 100 : 200;
        
        // Draw sender
        char line[200];
        snprintf(line, sizeof(line), "%s: %s", msg->sender, msg->message);
        draw_text(renderer, line, area->x + 5, area->y + y_offset, r, g, b);
        
        y_offset += 12;
    }
}

// ============================================================================
// HUD SYSTEM
// ============================================================================

void hud_update(HUDData* hud, struct World* world, EntityID player_id) {
    if (!hud || !world) return;
    
    // Get player data
    struct Transform* transform = entity_get_transform(world, player_id);
    struct Physics* physics = entity_get_physics(world, player_id);
    
    if (physics) {
        hud->speed = sqrtf(physics->velocity.x * physics->velocity.x + 
                          physics->velocity.y * physics->velocity.y + 
                          physics->velocity.z * physics->velocity.z);
    }
    
    if (transform) {
        hud->heading = atan2f(transform->rotation.y, transform->rotation.x) * 180.0f / M_PI;
    }
    
    // Set placeholder values (would come from actual game systems)
    hud->health = 100.0f;
    hud->shields = 85.0f;
    hud->fuel = 67.0f;
    strcpy(hud->target_name, "None");
    hud->target_distance = 0.0f;
    hud->target_hostile = false;
}

void hud_render(const HUDData* hud, SDL_Renderer* renderer, const UIRect* area) {
    if (!hud || !renderer || !area) return;
    
    // Draw HUD background
    draw_filled_rect(renderer, area, 20, 20, 40, 200);
    draw_outline_rect(renderer, area, 100, 150, 200);
    
    // Draw HUD elements
    char text[100];
    int x_offset = 20;
    
    // Speed
    snprintf(text, sizeof(text), "SPD: %.1f", hud->speed);
    draw_text(renderer, text, area->x + x_offset, area->y + 10, 255, 255, 255);
    x_offset += 100;
    
    // Heading
    snprintf(text, sizeof(text), "HDG: %.0f°", hud->heading);
    draw_text(renderer, text, area->x + x_offset, area->y + 10, 255, 255, 255);
    x_offset += 100;
    
    // Health
    snprintf(text, sizeof(text), "HP: %.0f%%", hud->health);
    uint8_t health_color = hud->health > 50 ? 255 : 200;
    draw_text(renderer, text, area->x + x_offset, area->y + 10, 
              255, health_color, health_color > 200 ? 0 : 0);
    x_offset += 100;
    
    // Shields
    snprintf(text, sizeof(text), "SLD: %.0f%%", hud->shields);
    draw_text(renderer, text, area->x + x_offset, area->y + 10, 0, 150, 255);
    x_offset += 100;
    
    // Fuel
    snprintf(text, sizeof(text), "FUEL: %.0f%%", hud->fuel);
    uint8_t fuel_color = hud->fuel > 25 ? 255 : 200;
    draw_text(renderer, text, area->x + x_offset, area->y + 10, 
              255, fuel_color, 0);
    x_offset += 100;
    
    // Target info
    if (strlen(hud->target_name) > 0 && strcmp(hud->target_name, "None") != 0) {
        snprintf(text, sizeof(text), "TGT: %s (%.1f)", hud->target_name, hud->target_distance);
        uint8_t tgt_r = hud->target_hostile ? 255 : 100;
        uint8_t tgt_g = hud->target_hostile ? 100 : 255;
        draw_text(renderer, text, area->x + x_offset, area->y + 10, tgt_r, tgt_g, 100);
    }
    
    // Status line
    draw_text(renderer, "STATUS: OPERATIONAL", area->x + 20, area->y + 30, 100, 255, 100);
}

// ============================================================================
// PUBLIC MESSAGE API
// ============================================================================

void cockpit_ui_add_message(CockpitUI* ui, const char* sender, const char* message, bool is_player) {
    if (!ui || !ui->initialized) return;
    comm_add_message(&ui->comm, sender, message, is_player);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (!renderer || !text) return;
    
    // Simple bitmap text rendering - just draw pixels for now
    // In a real implementation, you'd use SDL_ttf or a bitmap font
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    
    int char_width = 6;
    int char_height = 8;
    
    for (int i = 0; text[i] && i < 50; i++) {
        // Draw a simple rectangle for each character
        SDL_Rect char_rect = {x + i * char_width, y, char_width - 1, char_height};
        SDL_RenderDrawRect(renderer, &char_rect);
    }
}

void draw_filled_rect(SDL_Renderer* renderer, const UIRect* rect, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!renderer || !rect) return;
    
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_Rect sdl_rect = {rect->x, rect->y, rect->w, rect->h};
    SDL_RenderFillRect(renderer, &sdl_rect);
}

void draw_outline_rect(SDL_Renderer* renderer, const UIRect* rect, uint8_t r, uint8_t g, uint8_t b) {
    if (!renderer || !rect) return;
    
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_Rect sdl_rect = {rect->x, rect->y, rect->w, rect->h};
    SDL_RenderDrawRect(renderer, &sdl_rect);
}
