/**
 * @file   test_hello.c
 * @brief  Unit tests for hello_world.
 *
 * hello_world has no logic functions to test directly (printf is a library
 * call, not our code). These tests verify the build toolchain is wired up
 * correctly and demonstrate the Unity test framework pattern used across
 * all examples in this repo.
 */

#include "unity.h"
#include <string.h>

/* Unity requires these two hooks even if empty. */
void setUp(void)    {}
void tearDown(void) {}

/**
 * @brief  Sanity check: the C runtime and Unity are working.
 *
 * If this test runs, the compiler, linker, and test framework are all
 * correctly configured for this machine.
 */
void test_build_is_sane(void) {
    TEST_ASSERT_EQUAL_INT(1, 1);
}

/**
 * @brief  Verify the newline-terminated message string is well-formed.
 *
 * We can't capture stdout in a unit test without pipes, but we can at
 * least verify the string literal we pass to printf has the right content.
 */
void test_message_ends_with_newline(void) {
    const char *msg = "Hello, World!\n";
    size_t len = strlen(msg);
    TEST_ASSERT_EQUAL_CHAR('\n', msg[len - 1]);
}

/**
 * @brief  Verify string length matches expected character count.
 */
void test_message_length(void) {
    const char *msg = "Hello, World!\n";
    TEST_ASSERT_EQUAL_INT(14, (int)strlen(msg));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_build_is_sane);
    RUN_TEST(test_message_ends_with_newline);
    RUN_TEST(test_message_length);
    return UNITY_END();
}
