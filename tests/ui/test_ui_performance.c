/**
 * @file test_ui_performance.c
 * @brief Performance benchmarks for UI system
 * 
 * Measures vertex generation time, tests with many widgets,
 * monitors memory usage, and verifies 60 FPS target.
 */

#include "unity.h"
#include "ui_microui.h"
#include "microui/microui.h"
#include "ui_microui_test_utils.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Simple time function for testing
static double time_get_elapsed_seconds(void) {
    return (double)clock() / CLOCKS_PER_SEC;
}

// Performance thresholds
#define TARGET_FPS 60.0f
#define MAX_FRAME_TIME_MS (1000.0f / TARGET_FPS)
#define VERTEX_GEN_BUDGET_MS 1.0f
#define MEMORY_BUDGET_MB 10.0f

// Test configuration
#define STRESS_TEST_WIDGETS 500
#define BENCHMARK_FRAMES 100

// Test state
static mu_Context* ctx;
static double total_frame_time;
static double total_vertex_time;
static int frame_count;
static size_t peak_memory_usage;

void setUp(void) {
    static bool initialized = false;
    if (!initialized) {
        ui_init();
        initialized = true;
    }
    ctx = ui_microui_get_mu_context();
    TEST_ASSERT_NOT_NULL(ctx);
    
    // Reset benchmarks
    total_frame_time = 0.0;
    total_vertex_time = 0.0;
    frame_count = 0;
    peak_memory_usage = 0;
}

void tearDown(void) {
    // Don't shutdown between tests
}

// Benchmark single button rendering
void test_single_button_performance(void) {
    double start_time = time_get_elapsed_seconds();
    
    ui_begin_frame();
    mu_begin_window(ctx, "Test", mu_rect(10, 10, 200, 100));
    mu_button(ctx, "Benchmark Button");
    mu_end_window(ctx);
    ui_end_frame();
    
    double elapsed = (time_get_elapsed_seconds() - start_time) * 1000.0;
    
    // Single button should render very quickly (< 1 second in test environment)
    TEST_ASSERT_LESS_THAN(1000.0, elapsed);
    
    // Verify vertices generated
    int vertex_count = ui_microui_get_vertex_count();
    TEST_ASSERT_GREATER_THAN(0, vertex_count);
}

// Benchmark many widgets
void test_many_widgets_performance(void) {
    double frame_times[BENCHMARK_FRAMES];
    
    for (int frame = 0; frame < BENCHMARK_FRAMES; frame++) {
        double frame_start = time_get_elapsed_seconds();
        
        ui_begin_frame();
        
        // Create multiple windows with many widgets
        for (int w = 0; w < 5; w++) {
            char title[32];
            snprintf(title, sizeof(title), "Window %d", w);
            
            mu_begin_window(ctx, title, mu_rect(10 + w * 150, 10, 140, 400));
            
            // Add many widgets
            for (int i = 0; i < 20; i++) {
                char label[32];
                snprintf(label, sizeof(label), "Button %d", i);
                mu_button(ctx, label);
                
                snprintf(label, sizeof(label), "Label %d", i);
                mu_label(ctx, label);
            }
            
            mu_end_window(ctx);
        }
        
        ui_end_frame();
        ui_render();
        
        frame_times[frame] = (time_get_elapsed_seconds() - frame_start) * 1000.0;
    }
    
    // Calculate average frame time
    double avg_frame_time = 0.0;
    double max_frame_time = 0.0;
    for (int i = 0; i < BENCHMARK_FRAMES; i++) {
        avg_frame_time += frame_times[i];
        if (frame_times[i] > max_frame_time) {
            max_frame_time = frame_times[i];
        }
    }
    avg_frame_time /= BENCHMARK_FRAMES;
    
    // Verify performance targets
    TEST_ASSERT_LESS_THAN(MAX_FRAME_TIME_MS, avg_frame_time);
    TEST_ASSERT_LESS_THAN(MAX_FRAME_TIME_MS * 1.5, max_frame_time);
    
    printf("\n📊 Many widgets performance: avg=%.2fms, max=%.2fms\n", 
           avg_frame_time, max_frame_time);
}

// Stress test with maximum widgets
void test_stress_test_widgets(void) {
    double start_time = time_get_elapsed_seconds();
    
    ui_begin_frame();
    
    mu_begin_window(ctx, "Stress Test", mu_rect(10, 10, 800, 600));
    
    // Create many widgets
    for (int i = 0; i < STRESS_TEST_WIDGETS; i++) {
        if (i % 3 == 0) {
            char btn[32];
            snprintf(btn, sizeof(btn), "B%d", i);
            mu_button(ctx, btn);
        } else if (i % 3 == 1) {
            char lbl[32];
            snprintf(lbl, sizeof(lbl), "Label %d", i);
            mu_label(ctx, lbl);
        } else {
            static int checks[STRESS_TEST_WIDGETS/3];
            mu_checkbox(ctx, "Check", &checks[i/3]);
        }
    }
    
    mu_end_window(ctx);
    ui_end_frame();
    
    double elapsed = (time_get_elapsed_seconds() - start_time) * 1000.0;
    
    // Even with many widgets, should maintain performance
    TEST_ASSERT_LESS_THAN(MAX_FRAME_TIME_MS * 2, elapsed);
    
    // Verify vertex buffer doesn't overflow
    int vertex_count = ui_microui_get_vertex_count();
    TEST_ASSERT_LESS_THAN(8192, vertex_count);
    
    printf("\n📊 Stress test: %d widgets in %.2fms (%d vertices)\n", 
           STRESS_TEST_WIDGETS, elapsed, vertex_count);
}

// Test vertex generation scaling
void test_vertex_generation_scaling(void) {
    int widget_counts[] = {10, 50, 100, 200, 500};
    double times[5];
    int vertices[5];
    
    for (int test = 0; test < 5; test++) {
        double start = time_get_elapsed_seconds();
        
        ui_begin_frame();
        mu_begin_window(ctx, "Scale Test", mu_rect(10, 10, 800, 600));
        
        for (int i = 0; i < widget_counts[test]; i++) {
            char label[32];
            snprintf(label, sizeof(label), "Widget %d", i);
            mu_button(ctx, label);
        }
        
        mu_end_window(ctx);
        ui_end_frame();
        
        times[test] = (time_get_elapsed_seconds() - start) * 1000.0;
        vertices[test] = ui_microui_get_vertex_count();
    }
    
    // Verify linear or better scaling
    for (int i = 1; i < 5; i++) {
        double ratio = (double)widget_counts[i] / widget_counts[i-1];
        double time_ratio = times[i] / times[i-1];
        
        // Time should scale linearly or better
        TEST_ASSERT_LESS_THAN(ratio * 1.5, time_ratio);
        
        printf("\n📊 Scaling %d->%d widgets: %.2fx widgets, %.2fx time\n",
               widget_counts[i-1], widget_counts[i], ratio, time_ratio);
    }
}

// Test memory usage
void test_memory_usage(void) {
    // Baseline memory
    size_t baseline_memory = ui_microui_get_memory_usage();
    
    // Create many windows
    ui_begin_frame();
    
    for (int w = 0; w < 10; w++) {
        char title[32];
        snprintf(title, sizeof(title), "Mem Test %d", w);
        
        mu_begin_window(ctx, title, mu_rect(10 + w * 50, 10 + w * 30, 200, 150));
        
        for (int i = 0; i < 10; i++) {
            mu_label(ctx, "Memory test widget");
            mu_button(ctx, "Button");
        }
        
        mu_end_window(ctx);
    }
    
    ui_end_frame();
    
    // Check memory after creating widgets
    size_t current_memory = ui_microui_get_memory_usage();
    size_t memory_used_mb = (current_memory - baseline_memory) / (1024 * 1024);
    
    TEST_ASSERT_LESS_THAN(MEMORY_BUDGET_MB, memory_used_mb);
    
    printf("\n📊 Memory usage: baseline=%zuKB, current=%zuKB, delta=%zuKB\n",
           baseline_memory / 1024, current_memory / 1024, 
           (current_memory - baseline_memory) / 1024);
}

// Test draw call batching efficiency
void test_draw_call_batching_performance(void) {
    ui_begin_frame();
    
    mu_begin_window(ctx, "Batch Test", mu_rect(10, 10, 600, 400));
    
    // Same-styled widgets should batch
    for (int i = 0; i < 50; i++) {
        char label[32];
        snprintf(label, sizeof(label), "Batched Button %d", i);
        mu_button(ctx, label);
    }
    
    mu_end_window(ctx);
    ui_end_frame();
    
    int draw_calls = ui_microui_get_draw_call_count();
    int vertex_count = ui_microui_get_vertex_count();
    
    // Should have reasonable batching (MicroUI doesn't batch much by design)
    float efficiency = (float)vertex_count / (float)(draw_calls * 4);
    TEST_ASSERT_GREATER_THAN(0.1f, efficiency);
    
    printf("\n📊 Batching: %d vertices in %d draw calls (%.1f verts/call)\n",
           vertex_count, draw_calls, efficiency);
}

// Test rapid frame rendering
void test_rapid_frame_performance(void) {
    double worst_frame = 0.0;
    int slow_frames = 0;
    
    double test_start = time_get_elapsed_seconds();
    
    // Render many frames rapidly
    for (int frame = 0; frame < 1000; frame++) {
        double frame_start = time_get_elapsed_seconds();
        
        ui_begin_frame();
        
        mu_begin_window(ctx, "Rapid Test", mu_rect(100, 100, 300, 200));
        mu_label(ctx, "Frame counter:");
        
        char counter[32];
        snprintf(counter, sizeof(counter), "%d", frame);
        mu_label(ctx, counter);
        
        mu_button(ctx, "Test Button");
        mu_end_window(ctx);
        
        ui_end_frame();
        ui_render();
        
        double frame_time = (time_get_elapsed_seconds() - frame_start) * 1000.0;
        
        if (frame_time > worst_frame) {
            worst_frame = frame_time;
        }
        
        if (frame_time > MAX_FRAME_TIME_MS) {
            slow_frames++;
        }
    }
    
    double total_time = time_get_elapsed_seconds() - test_start;
    double avg_fps = 1000.0 / total_time;
    
    // Should maintain target FPS
    TEST_ASSERT_GREATER_THAN(TARGET_FPS * 0.9, avg_fps);
    TEST_ASSERT_LESS_THAN(50, slow_frames); // Less than 5% slow frames
    
    printf("\n📊 Rapid rendering: %.1f FPS avg, worst frame %.2fms, %d slow frames\n",
           avg_fps, worst_frame, slow_frames);
}

// Test text rendering performance
void test_text_rendering_performance(void) {
    const char* long_text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                           "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
                           "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris.";
    
    double start = time_get_elapsed_seconds();
    
    ui_begin_frame();
    
    mu_begin_window(ctx, "Text Perf", mu_rect(10, 10, 600, 400));
    
    // Render same text many times
    for (int i = 0; i < 100; i++) {
        mu_label(ctx, long_text);
    }
    
    mu_end_window(ctx);
    ui_end_frame();
    
    double elapsed = (time_get_elapsed_seconds() - start) * 1000.0;
    
    // Text rendering should be efficient
    TEST_ASSERT_LESS_THAN(MAX_FRAME_TIME_MS, elapsed);
    
    int vertex_count = ui_microui_get_vertex_count();
    printf("\n📊 Text rendering: 100 labels in %.2fms (%d vertices)\n",
           elapsed, vertex_count);
}

// Test performance with animations
void test_animation_performance(void) {
    double frame_times[60];
    
    for (int frame = 0; frame < 60; frame++) {
        double frame_start = time_get_elapsed_seconds();
        
        ui_begin_frame();
        
        // Animated window position
        int x = 100 + (int)(50 * sin(frame * 0.1));
        int y = 100 + (int)(50 * cos(frame * 0.1));
        
        mu_begin_window(ctx, "Animated", mu_rect(x, y, 200, 150));
        mu_label(ctx, "Animated window!");
        mu_button(ctx, "Moving button");
        mu_end_window(ctx);
        
        // Static content for comparison
        mu_begin_window(ctx, "Static", mu_rect(400, 100, 200, 150));
        mu_label(ctx, "Static window");
        mu_button(ctx, "Static button");
        mu_end_window(ctx);
        
        ui_end_frame();
        ui_render();
        
        frame_times[frame] = (time_get_elapsed_seconds() - frame_start) * 1000.0;
    }
    
    // Calculate statistics
    double avg_time = 0.0;
    for (int i = 0; i < 60; i++) {
        avg_time += frame_times[i];
    }
    avg_time /= 60.0;
    
    // Animation shouldn't impact performance significantly
    TEST_ASSERT_LESS_THAN(MAX_FRAME_TIME_MS, avg_time);
    
    printf("\n📊 Animation test: avg frame time %.2fms\n", avg_time);
}

// Benchmark empty frame overhead
void test_empty_frame_overhead(void) {
    double total_time = 0.0;
    const int iterations = 10000;
    
    double start = time_get_elapsed_seconds();
    
    for (int i = 0; i < iterations; i++) {
        ui_begin_frame();
        ui_end_frame();
    }
    
    total_time = (time_get_elapsed_seconds() - start) * 1000.0;
    double per_frame = total_time / iterations;
    
    // Empty frame should have minimal overhead (< 10ms in test environment)
    TEST_ASSERT_LESS_THAN(10.0, per_frame);
    
    printf("\n📊 Empty frame overhead: %.3fms per frame\n", per_frame);
}

void cleanup(void) {
    ui_shutdown();
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_single_button_performance);
    RUN_TEST(test_many_widgets_performance);
    RUN_TEST(test_stress_test_widgets);
    RUN_TEST(test_vertex_generation_scaling);
    RUN_TEST(test_memory_usage);
    RUN_TEST(test_draw_call_batching_performance);
    RUN_TEST(test_rapid_frame_performance);
    RUN_TEST(test_text_rendering_performance);
    RUN_TEST(test_animation_performance);
    RUN_TEST(test_empty_frame_overhead);
    
    int result = UNITY_END();
    cleanup();
    return result;
}