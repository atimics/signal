# Sprint 14 Research: Asynchronous AI System

This document contains the research and best practices for integrating an asynchronous AI system.

## Implementation Guide & Best Practices

This sprint introduces multi-threading to the engine. Adhering to best practices is critical to avoid race conditions and deadlocks.

### 1. Core Architecture: Asynchronous Job System

We will implement a simple producer-consumer model:
-   **Main Thread (Producer):** The `ai_system_update` will identify entities needing AI updates and push "AI Tasks" into a shared, thread-safe queue. It will not wait for a result.
-   **AI Worker Thread (Consumer):** A new thread will continuously pull tasks from the queue, run the `gemma.cpp` inference, and push the "AI Result" into a separate thread-safe results queue.
-   **Resource**: [Introduction to Job Systems (Gist by Rich Geldreich)](https://gist.github.com/richgel999/6955960)

### 2. Threading with C11 `threads.h`

We will use the standard C11 threading library for portability. This is a modern, clean, and standard way to handle threading in C.
-   **`thrd_t`**: Represents the thread handle.
-   **`thrd_create()`**: Spawns the new thread.
-   **`thrd_join()`**: Waits for the thread to finish during application cleanup.
-   **Resource**: [C11 `threads.h` Reference (cppreference.com)](https://en.cppreference.com/w/c/thread)
