// tests/test_core_math.c
#include "vendor/unity.h"
#include "core.h"

// Test function for vector addition
void test_vector_addition(void) {
    Vector3 a = {1.0f, 2.0f, 3.0f};
    Vector3 b = {4.0f, 5.0f, 6.0f};
    Vector3 result = vec3_add(a, b);
    
    TEST_ASSERT_EQUAL_FLOAT(5.0f, result.x);
    TEST_ASSERT_EQUAL_FLOAT(7.0f, result.y);
    TEST_ASSERT_EQUAL_FLOAT(9.0f, result.z);
}

// Test function for vector dot product
void test_vector_dot_product(void) {
    Vector3 a = {1.0f, 2.0f, 3.0f};
    Vector3 b = {4.0f, 5.0f, 6.0f};
    float result = vec3_dot(a, b);

    // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
    TEST_ASSERT_EQUAL_FLOAT(32.0f, result);
}

// Test function for vector normalization
void test_vector_normalize(void) {
    Vector3 a = {3.0f, 4.0f, 0.0f}; // Length is 5
    Vector3 result = vec3_normalize(a);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.6f, result.x); // 3/5
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.8f, result.y); // 4/5
    TEST_ASSERT_EQUAL_FLOAT(0.0f, result.z);
}

// This function is called by the test runner to execute all tests in this suite.
void suite_core_math(void) {
    RUN_TEST(test_vector_addition);
    RUN_TEST(test_vector_dot_product);
    RUN_TEST(test_vector_normalize);
}
