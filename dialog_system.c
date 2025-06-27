#include "cgame.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global dialog state
static struct DialogSession g_active_dialog = {0};

bool dialog_start_conversation(struct DialogSession* dialog, struct Entity* entity, const char* player_message) {
    if (!dialog || !entity) return false;
    
    printf("💬 Starting conversation with %s\n", entity->name ? entity->name : "Unknown Entity");
    
    // Initialize dialog session
    memset(dialog, 0, sizeof(struct DialogSession));
    dialog->speaking_entity = entity;
    dialog->is_speaking = true;
    dialog->speaking_start_time = 0.0f; // Will be set by game timer
    
    // Add player message to history
    dialog_add_to_history(dialog, "Player", player_message);
    
    // Build conversation context
    char* context = dialog_build_conversation_context(dialog);
    
    // Create a rich prompt like your CosyWorld system
    char full_prompt[2048];
    snprintf(full_prompt, sizeof(full_prompt),
        "You are %s, a %s pilot in a dangerous space sector.\n"
        "Position: (%.1f, %.1f, %.1f)\n"
        "Current Status: %s\n"
        "Personality: %s\n\n"
        "Recent conversation:\n%s\n"
        "Player just said: \"%s\"\n\n"
        "Respond naturally as this character would. Keep response under 150 characters for real-time dialog:\n",
        entity->name ? entity->name : "Unknown Pilot",
        entity->type == ENTITY_AI_FIGHTER ? "fighter" :
        entity->type == ENTITY_AI_TRADER ? "merchant" :
        entity->type == ENTITY_AI_EXPLORER ? "explorer" : "civilian",
        entity->position.x, entity->position.y, entity->position.z,
        entity->current_state == ENTITY_STATE_IDLE ? "idle" :
        entity->current_state == ENTITY_STATE_PATROLLING ? "on patrol" :
        entity->current_state == ENTITY_STATE_PURSUING ? "in pursuit" : "active",
        entity->personality && entity->personality->base_prompt ? 
            entity->personality->base_prompt : "Professional and cautious",
        context ? context : "No prior conversation",
        player_message
    );
    
    // Start streaming AI response
    printf("🧠 Requesting AI response...\n");
    
    // For real-time feel, use streaming API
    struct LlamaInference* llama_engine = ai_get_llama_engine();
    if (llama_is_ready(llama_engine)) {
        llama_generate_stream(llama_engine, full_prompt, 100, 
                             dialog_stream_token_callback, dialog);
    } else {
        // Fallback to regular generation
        char* response = llama_generate(llama_engine, full_prompt, 100);
        if (response) {
            dialog->current_response = response;
            dialog->response_length = strlen(response);
            dialog->response_complete = true;
        }
    }
    
    if (context) free(context);
    return true;
}

void dialog_stream_token_callback(const char* token, void* user_data) {
    struct DialogSession* dialog = (struct DialogSession*)user_data;
    if (!dialog || !token) return;
    
    // Append token to current response
    if (!dialog->current_response) {
        dialog->current_response = strdup(token);
        dialog->response_length = strlen(token);
    } else {
        int old_len = strlen(dialog->current_response);
        int token_len = strlen(token);
        dialog->current_response = realloc(dialog->current_response, old_len + token_len + 1);
        strcpy(dialog->current_response + old_len, token);
        dialog->response_length = old_len + token_len;
    }
    
    printf("📝 Token received: \"%s\" (total: %d chars)\n", token, dialog->response_length);
}

void dialog_update(struct DialogSession* dialog, float deltaTime) {
    if (!dialog || !dialog->is_speaking) return;
    
    // Update typewriter effect timer
    dialog->char_timer += deltaTime;
    
    // Display characters at typewriter speed (20 chars/second)
    if (dialog->char_timer >= 0.05f) {
        if (dialog->display_chars < dialog->response_length) {
            dialog->display_chars++;
        }
        dialog->char_timer = 0.0f;
    }
    
    // Check if response is complete
    if (dialog->response_complete && dialog->display_chars >= dialog->response_length) {
        printf("✅ Dialog response complete: \"%s\"\n", dialog->current_response);
        dialog_add_to_history(dialog, dialog->speaking_entity->name, dialog->current_response);
        dialog->is_speaking = false;
    }
}

void dialog_end_conversation(struct DialogSession* dialog) {
    if (!dialog) return;
    
    printf("👋 Ending conversation\n");
    
    // Cleanup
    if (dialog->current_response) {
        free(dialog->current_response);
    }
    
    for (int i = 0; i < dialog->history_count; i++) {
        if (dialog->conversation_history[i]) {
            free(dialog->conversation_history[i]);
        }
    }
    
    memset(dialog, 0, sizeof(struct DialogSession));
}

void dialog_add_to_history(struct DialogSession* dialog, const char* speaker, const char* message) {
    if (!dialog || !speaker || !message) return;
    
    // Create history entry
    char history_entry[512];
    snprintf(history_entry, sizeof(history_entry), "%s: %s", speaker, message);
    
    // Add to history (circular buffer)
    if (dialog->history_count < 10) {
        dialog->conversation_history[dialog->history_count] = strdup(history_entry);
        dialog->history_count++;
    } else {
        // Replace oldest entry
        if (dialog->conversation_history[0]) free(dialog->conversation_history[0]);
        for (int i = 0; i < 9; i++) {
            dialog->conversation_history[i] = dialog->conversation_history[i + 1];
        }
        dialog->conversation_history[9] = strdup(history_entry);
    }
}

char* dialog_build_conversation_context(struct DialogSession* dialog) {
    if (!dialog || dialog->history_count == 0) return NULL;
    
    int total_length = 0;
    for (int i = 0; i < dialog->history_count; i++) {
        if (dialog->conversation_history[i]) {
            total_length += strlen(dialog->conversation_history[i]) + 1;
        }
    }
    
    char* context = malloc(total_length + 1);
    context[0] = '\0';
    
    for (int i = 0; i < dialog->history_count; i++) {
        if (dialog->conversation_history[i]) {
            strcat(context, dialog->conversation_history[i]);
            strcat(context, "\n");
        }
    }
    
    return context;
}

bool dialog_should_display_char(struct DialogSession* dialog, float deltaTime) {
    if (!dialog) return false;
    (void)deltaTime; // Suppress unused parameter warning
    return dialog->display_chars < dialog->response_length;
}

float dialog_get_speaking_progress(struct DialogSession* dialog) {
    if (!dialog || dialog->response_length == 0) return 0.0f;
    return (float)dialog->display_chars / (float)dialog->response_length;
}

// Dialog response callback for AI engine integration
void dialog_on_response_complete(struct Entity* entity, char* full_response) {
    if (!entity || !full_response) return;
    
    printf("🎭 %s says: \"%s\"\n", 
           entity->name ? entity->name : "Unknown", full_response);
    
    // This would be called by the AI engine when a dialog task completes
    // The response is already handled by the streaming system above
}
