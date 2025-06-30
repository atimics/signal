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

### 3. Thread-Safe Communication: Mutexes & Condition Variables

To prevent data corruption, the shared task and result queues **must** be protected.
-   **`mtx_t` (Mutex):** We will create a mutex to lock the queues during read/write operations. A thread must acquire the lock, perform its operation, and then release the lock.
-   **`cnd_t` (Condition Variable):** The AI worker thread will use a condition variable to sleep efficiently when the task queue is empty. The main thread will signal this variable when it adds a new task, waking the worker up. This is far more efficient than busy-waiting or polling.
-   **Resource**: [Condition Variables (GeeksforGeeks)](https://www.geeksforgeeks.org/condition-variables-in-c/)

### 4. The Asynchronous Workflow

1.  **`ai_system_update` (Main Thread - Producer):**
    -   `mtx_lock(&task_queue_mutex)`
    -   `queue_push(&task_queue, new_task)`
    -   `cnd_signal(&task_queue_condvar)` // Wake up the worker if it's sleeping
    -   `mtx_unlock(&task_queue_mutex)`
2.  **`ai_worker_function` (AI Thread - Consumer):**
    -   `mtx_lock(&task_queue_mutex)`
    -   `while (is_queue_empty(&task_queue)) { cnd_wait(&task_queue_condvar, &task_queue_mutex); }` // Atomically unlocks mutex and waits
    -   `task = queue_pop(&task_queue)`
    -   `mtx_unlock(&task_queue_mutex)`
    -   `result = gemma_inference(task.prompt)` // This is the expensive call, performed outside the lock
    -   `mtx_lock(&result_queue_mutex)`
    -   `queue_push(&result_queue, result)`
    -   `mtx_unlock(&result_queue_mutex)`
3.  **`ai_results_system_update` (Main Thread - Consumer):**
    -   `mtx_lock(&result_queue_mutex)`
    -   `while (!is_queue_empty(&result_queue)) { result = queue_pop(&result_queue); apply_ai_result(result); }`
    -   `mtx_unlock(&result_queue_mutex)`
-   **Best Practice**: Keep the time spent inside a mutex lock as short as possible. The expensive work (inference) should always happen *outside* the lock to avoid blocking other threads.
