// tests/core/test_math.c
#include "../support/test_utilities.h"
#include "../../src/core.h"

// Test function for vector addition
void test_vector_addition(void)
{
    Vector3 a = { 1.0f, 2.0f, 3.0f };
    Vector3 b = { 4.0f, 5.0f, 6.0f };
    Vector3 result = vector3_add(a, b);

    TEST_ASSERT_EQUAL_FLOAT(5.0f, result.x);
    TEST_ASSERT_EQUAL_FLOAT(7.0f, result.y);
    TEST_ASSERT_EQUAL_FLOAT(9.0f, result.z);
}

// Test function for vector subtraction
void test_vector_subtraction(void)
{
    Vector3 a = { 5.0f, 7.0f, 9.0f };
    Vector3 b = { 1.0f, 2.0f, 3.0f };
    Vector3 result = vector3_subtract(a, b);

    TEST_ASSERT_EQUAL_FLOAT(4.0f, result.x);
    TEST_ASSERT_EQUAL_FLOAT(5.0f, result.y);
    TEST_ASSERT_EQUAL_FLOAT(6.0f, result.z);
}

// Test function for vector multiplication by scalar
void test_vector_multiply(void)
{
    Vector3 a = { 1.0f, 2.0f, 3.0f };
    Vector3 result = vector3_multiply(a, 2.0f);

    TEST_ASSERT_EQUAL_FLOAT(2.0f, result.x);
    TEST_ASSERT_EQUAL_FLOAT(4.0f, result.y);
    TEST_ASSERT_EQUAL_FLOAT(6.0f, result.z);
}

// Test function for vector normalization
void test_vector_normalize(void)
{
    Vector3 a = { 3.0f, 4.0f, 0.0f };  // Length is 5
    Vector3 result = vector3_normalize(a);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.6f, result.x);  // 3/5
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.8f, result.y);  // 4/5
    TEST_ASSERT_EQUAL_FLOAT(0.0f, result.z);
}

// Test function for vector length
void test_vector_length(void)
{
    Vector3 a = { 3.0f, 4.0f, 0.0f };  // Length should be 5
    float result = vector3_length(a);

    TEST_ASSERT_EQUAL_FLOAT(5.0f, result);
}

// Test function for quaternion rotate vector - identity rotation
void test_quaternion_rotate_vector_identity(void)
{
    Quaternion identity = { 0.0f, 0.0f, 0.0f, 1.0f };
    Vector3 v = { 1.0f, 2.0f, 3.0f };
    Vector3 result = quaternion_rotate_vector(identity, v);
    
    // Identity quaternion should not change the vector
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, result.x);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, result.y);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 3.0f, result.z);
}

// Test function for quaternion rotate vector - 90 degree rotation around Y
void test_quaternion_rotate_vector_90_deg_y(void)
{
    // 90 degree rotation around Y axis
    Quaternion q = { 0.0f, 0.707f, 0.0f, 0.707f };
    Vector3 v = { 1.0f, 0.0f, 0.0f };
    Vector3 result = quaternion_rotate_vector(q, v);
    
    // Rotating (1,0,0) 90 degrees around Y should give (0,0,-1)
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, result.x);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, result.y);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -1.0f, result.z);
}

// Test function for quaternion rotate vector - 180 degree rotation around Z
void test_quaternion_rotate_vector_180_deg_z(void)
{
    // 180 degree rotation around Z axis
    Quaternion q = { 0.0f, 0.0f, 1.0f, 0.0f };
    Vector3 v = { 1.0f, 0.0f, 0.0f };
    Vector3 result = quaternion_rotate_vector(q, v);
    
    // Rotating (1,0,0) 180 degrees around Z should give (-1,0,0)
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -1.0f, result.x);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, result.y);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, result.z);
}

// This function is called by the test runner to execute all tests in this suite.
void suite_core_math(void)
{
    RUN_TEST(test_vector_addition);
    RUN_TEST(test_vector_subtraction);
    RUN_TEST(test_vector_multiply);
    RUN_TEST(test_vector_normalize);
    RUN_TEST(test_vector_length);
    RUN_TEST(test_quaternion_rotate_vector_identity);
    RUN_TEST(test_quaternion_rotate_vector_90_deg_y);
    RUN_TEST(test_quaternion_rotate_vector_180_deg_z);
}
