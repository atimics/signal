#ifndef LLAMA_INTEGRATION_H
#define LLAMA_INTEGRATION_H

#include <stdbool.h>
#include <stdint.h>

// Lightweight wrapper around llama.cpp for our game
// This provides a simple C interface to the llama.cpp library

struct LlamaInference {
    void* model;        // llama_model*
    void* context;      // llama_context*
    int32_t* tokens;    // Token buffer
    int n_tokens;       // Current token count
    int max_tokens;     // Maximum tokens
    bool is_loaded;
    char* model_path;
};

// Initialize the inference engine
bool llama_init(struct LlamaInference* inference, const char* model_path, int max_context_size);

// Shutdown and cleanup
void llama_shutdown(struct LlamaInference* inference);

// Generate text from a prompt
// Returns allocated string that must be freed by caller
char* llama_generate(struct LlamaInference* inference, const char* prompt, int max_tokens);

// Generate text with callback for streaming responses
// Useful for real-time dialog where we want to start showing response immediately
typedef void (*LlamaStreamCallback)(const char* token, void* user_data);
bool llama_generate_stream(struct LlamaInference* inference, const char* prompt, 
                          int max_tokens, LlamaStreamCallback callback, void* user_data);

// Check if model is loaded and ready
bool llama_is_ready(struct LlamaInference* inference);

// Performance monitoring
float llama_get_last_inference_time(struct LlamaInference* inference);
int llama_get_token_count(struct LlamaInference* inference);

#endif
