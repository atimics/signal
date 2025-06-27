#ifndef DIALOG_SYSTEM_H
#define DIALOG_SYSTEM_H

#include <stdbool.h>
#include "entity.h"

// Real-time dialog system for interactive conversations
// Inspired by your CosyWorld chat interfaces

struct DialogSession {
    struct Entity* speaking_entity;
    char* current_response;
    int response_length;
    bool is_speaking;
    bool response_complete;
    float speaking_start_time;
    
    // Dialog state
    char* conversation_history[10];
    int history_count;
    
    // Visual presentation
    int display_chars;  // For typewriter effect
    float char_timer;
};

// Dialog System API
bool dialog_start_conversation(struct DialogSession* dialog, struct Entity* entity, const char* player_message);
void dialog_update(struct DialogSession* dialog, float deltaTime);
void dialog_end_conversation(struct DialogSession* dialog);

// Real-time streaming callbacks
void dialog_stream_token_callback(const char* token, void* user_data);
void dialog_on_response_complete(struct Entity* entity, char* full_response);

// Conversation management
void dialog_add_to_history(struct DialogSession* dialog, const char* speaker, const char* message);
char* dialog_build_conversation_context(struct DialogSession* dialog);

// Visual effects
bool dialog_should_display_char(struct DialogSession* dialog, float deltaTime);
float dialog_get_speaking_progress(struct DialogSession* dialog);

#endif
