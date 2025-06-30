# Sprint 14: Asynchronous AI System Integration

**Duration**: 2 weeks
**Priority**: High
**Dependencies**: Sprint 13 (UI Finalization)

## Sprint Goal
Integrate the `gemma.cpp` inference engine into the CGame engine using an asynchronous, multi-threaded job system. This will allow the engine to perform expensive AI calculations without blocking the main render loop, maintaining a smooth 60fps frame rate.

## Current State
- ✅ AI system architecture designed, but implementation is synchronous and uses mock functions.
- ✅ Core engine systems (ECS, rendering, physics) are functional.
- ❌ No actual neural inference is being performed.
- ❌ AI calculations would currently block the main thread and cause severe frame drops.

## Target State
- ✅ A dedicated worker thread for AI inference is created on startup.
- ✅ A thread-safe task queue is implemented for submitting AI jobs from the main thread.
- ✅ The `gemma.cpp` library is integrated and performs inference on the worker thread.
- ✅ AI decisions are returned to the main thread via a results queue and applied to entities.
- ✅ The main render loop remains non-blocking and maintains 60fps during AI inference.

---

## Implementation Guide & Best Practices

This sprint introduces multi-threading to the engine. Adhering to best practices is critical to avoid race conditions and deadlocks.

### 1. Core Architecture: Asynchronous Job System

We will implement a simple producer-consumer model:
- **Main Thread (Producer):** The `ai_system_update` will identify entities needing AI updates and push "AI Tasks" into a shared queue. It will not wait for a result.
- **AI Worker Thread (Consumer):** A new thread will continuously pull tasks from the queue, run the `gemma.cpp` inference, and push the "AI Result" into a separate results queue.

### 2. Threading with C11 `threads.h`

We will use the standard C11 threading library for portability.
- **`thrd_t`**: Represents the thread handle.
- **`thrd_create()`**: Spawns the new thread.
- **`thrd_join()`**: Waits for the thread to finish during application cleanup.

### 3. Thread-Safe Communication: Mutexes & Condition Variables

To prevent data corruption, the shared task and result queues **must** be protected.
- **`mtx_t` (Mutex):** We will create a mutex to lock the queues during read/write operations. A thread must acquire the lock, perform its operation, and then release the lock.
- **`cnd_t` (Condition Variable):** The AI worker thread will use a condition variable to sleep efficiently when the task queue is empty. The main thread will signal this variable when it adds a new task, waking the worker up.

### 4. The Asynchronous Workflow

1.  **`ai_system_update` (Main Thread):**
    - `mtx_lock(&task_queue_mutex)`
    - `queue_push(&task_queue, new_task)`
    - `cnd_signal(&task_queue_condvar)`
    - `mtx_unlock(&task_queue_mutex)`
2.  **`ai_worker_function` (AI Thread):**
    - `mtx_lock(&task_queue_mutex)`
    - `while (is_queue_empty(&task_queue)) { cnd_wait(&task_queue_condvar, &task_queue_mutex); }`
    - `task = queue_pop(&task_queue)`
    - `mtx_unlock(&task_queue_mutex)`
    - `result = gemma_inference(task.prompt)` // Expensive call
    - `mtx_lock(&result_queue_mutex)`
    - `queue_push(&result_queue, result)`
    - `mtx_unlock(&result_queue_mutex)`
3.  **`ai_results_system_update` (Main Thread):**
    - `mtx_lock(&result_queue_mutex)`
    - `while (!is_queue_empty(&result_queue)) { result = queue_pop(&result_queue); apply_ai_result(result); }`
    - `mtx_unlock(&result_queue_mutex)`

---

## Tasks

### Task 14.1: Job System Foundation
**Estimated**: 2 days
**Files**: `src/ai_job_system.c`, `src/ai_job_system.h`

#### Acceptance Criteria
- [ ] Create a generic, thread-safe queue implementation for tasks and results.
- [ ] Implement functions to initialize, push to, and pop from the queue using a mutex for protection.
- [ ] Create a function to spawn the AI worker thread using `thrd_create`.
- [ ] The worker thread should loop indefinitely, waiting on a condition variable.
- [ ] Implement a shutdown mechanism to signal the worker thread to exit cleanly.

### Task 14.2: Integrate `gemma.cpp`
**Estimated**: 2 days
**Files**: `Makefile`, `src/gemma_integration.c`

#### Acceptance Criteria
- [ ] Download and add the `gemma.cpp` source to a `vendor/` directory.
- [ ] Update the `Makefile` to correctly compile `gemma.cpp` and link it with the main executable.
- [ ] Create a simple wrapper function `gemma_init()` and `gemma_generate(prompt)`.
- [ ] The `gemma.cpp` model and context should be loaded and managed exclusively by the AI worker thread to ensure thread safety.

### Task 14.3: Asynchronous AI Task Submission
**Estimated**: 3 days
**Files**: `src/systems.c`, `src/ai_job_system.c`

#### Acceptance Criteria
- [ ] Modify `ai_system_update` to be the "producer".
- [ ] It should identify an entity needing an AI update.
- [ ] It should generate a prompt based on the entity's state.
- [ ] It should push an `AITask` struct (containing entity ID and prompt) to the thread-safe task queue.
- [ ] The AI worker thread function should be updated to:
    - [ ] Dequeue a task.
    - [ ] Call `gemma_generate()` with the task's prompt.
    - [ ] Push an `AIResult` struct (containing entity ID and response string) to the results queue.

### Task 14.4: Process AI Results
**Estimated**: 2 days
**Files**: `src/systems.c`

#### Acceptance Criteria
- [ ] Create a new system, `ai_results_system_update`, that runs on the main thread.
- [ ] This system checks the results queue on every frame.
- [ ] If a result is available, it dequeues it and applies the AI's decision to the corresponding entity (e.g., updating its `AI` component state).
- [ ] The system must handle the case where no result is available without blocking.

### Task 14.5: Debug UI
**Estimated**: 1 day
**Files**: `src/ui.c`

#### Acceptance Criteria
- [ ] Add a new "AI Debug" panel to the Nuklear UI.
- [ ] Display the current size of the AI task queue and results queue.
- [ ] Show the status of the AI worker thread (e.g., "Idle", "Processing Task for Entity X").
- [ ] Display the last prompt sent and the last response received.

---

## Definition of Done

- [ ] AI inference runs on a separate thread without causing frame rate drops.
- [ ] The main thread can submit AI tasks and receive results asynchronously.
- [ ] `gemma.cpp` is successfully integrated and generating text responses.
- [ ] Entity behavior is visibly updated based on AI-generated results.
- [ ] The system is thread-safe, with no deadlocks or race conditions observed during testing.
- [ ] The debug UI provides clear insight into the state of the asynchronous AI system.
