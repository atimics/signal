// tests/sprint_10_5/test_task_2.c
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

// Forward-declare the function we are testing (to be implemented in src/assets.c)
#include "assets.h" // We need the full Mesh definition

// Helper to check if a pointer is to the heap (not foolproof, but good enough for this test)
// A more robust test would involve overriding malloc, which is too complex for this sprint.
bool is_heap_pointer(void* p) {
    // A simple check: stack pointers are usually very high in the address space.
    // This is not guaranteed but is a reasonable heuristic for a test.
    char stack_var;
    return p < (void*)&stack_var;
}

void test_mesh_parser_stability() {
    printf("Running Test: test_mesh_parser_stability\n");

    Mesh small_mesh = {0};
    Mesh large_mesh = {0};

    // --- Test Case 1: Parse small mesh ---
    printf("  Case 1: Parse small_mesh.cobj...\n");
    bool small_parsed = parse_obj_file("tests/sprint_10_5/small_mesh.cobj", &small_mesh);
    assert(small_parsed == true);
    assert(small_mesh.vertex_count == 3);
    assert(small_mesh.index_count == 3);
    assert(small_mesh.vertices != NULL);
    assert(small_mesh.indices != NULL);
    // Verify memory is from the heap, not the stack
    assert(is_heap_pointer(small_mesh.vertices));
    assert(is_heap_pointer(small_mesh.indices));
    printf("    ... PASSED\n");


    // --- Test Case 2: Parse large mesh ---
    printf("  Case 2: Parse large_mesh.cobj...\n");
    bool large_parsed = parse_obj_file("tests/sprint_10_5/large_mesh.cobj", &large_mesh);
    assert(large_parsed == true);
    assert(large_mesh.vertex_count == (20000 - 2) * 3); // Each face adds 3 vertices
    assert(large_mesh.index_count == (20000 - 2) * 3);
    assert(large_mesh.vertices != NULL);
    assert(large_mesh.indices != NULL);
    assert(is_heap_pointer(large_mesh.vertices));
    assert(is_heap_pointer(large_mesh.indices));
    printf("    ... PASSED\n");

    // --- Test Case 3: Handle non-existent file ---
    printf("  Case 3: Handle non-existent file...\n");
    Mesh non_existent_mesh = {0};
    bool non_existent_parsed = parse_obj_file("tests/sprint_10_5/no_such_file.cobj", &non_existent_mesh);
    assert(non_existent_parsed == false);
    assert(non_existent_mesh.vertex_count == 0);
    assert(non_existent_mesh.vertices == NULL);
    printf("    ... PASSED\n");

    // Cleanup
    free(small_mesh.vertices);
    free(small_mesh.indices);
    free(large_mesh.vertices);
    free(large_mesh.indices);

    printf("Test Finished: test_mesh_parser_stability\n\n");
}

// We will replace the main in runner.c to call our tests
// in the correct order.
