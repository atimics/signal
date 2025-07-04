/**
 * @file ui_microui_adapter.c
 * @brief Adapter functions for scene UI to use Microui instead of Nuklear
 */

#include "ui_scene.h"
#include "ui_microui.h"
#include "ui_api.h"
#include "microui/microui.h"
#include "core.h"
#include <string.h>
#include <stdio.h>

// This file serves as a bridge between the UI system and Microui
// The actual scene rendering is implemented in ui_scene.c

// HUD system removed - UI rendering handled through scene-specific UI modules