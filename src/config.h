/**
 * @file config.h
 * @brief Configuration management system
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

// Configuration structure
typedef struct {
    char startup_scene[64];
    bool auto_start;
    float master_volume;
    bool fullscreen;
    int window_width;
    int window_height;
} GameConfig;

// Config API
bool config_init(void);
void config_shutdown(void);
GameConfig* config_get(void);
bool config_save(void);
bool config_load(void);

// Specific getters/setters for common config values
const char* config_get_startup_scene(void);
void config_set_startup_scene(const char* scene_name);
bool config_get_auto_start(void);
void config_set_auto_start(bool auto_start);

#endif // CONFIG_H
