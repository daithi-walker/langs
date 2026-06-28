/**
 * @file   test_physics.c
 * @brief  Unit tests for bounce physics (ball_update, ball_speed).
 */

#include "unity.h"
#include "../physics.h"
#include <math.h>

void setUp(void)    {}
void tearDown(void) {}

void test_normal_movement(void) {
    Ball b = {100.0f, 100.0f, 4.0f, 3.0f};
    ball_update(&b, 10, 800, 600);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 104.0f, b.x);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 103.0f, b.y);
}

void test_left_wall_reflection(void) {
    Ball b = {10.0f, 100.0f, -5.0f, 3.0f};
    ball_update(&b, 10, 800, 600);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.0f, b.vx);
}

void test_right_wall_reflection(void) {
    Ball b = {794.0f, 100.0f, 5.0f, 3.0f};
    ball_update(&b, 10, 800, 600);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -5.0f, b.vx);
}

void test_top_wall_reflection(void) {
    Ball b = {100.0f, 8.0f, 4.0f, -5.0f};
    ball_update(&b, 10, 800, 600);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.0f, b.vy);
}

void test_bottom_wall_reflection(void) {
    Ball b = {100.0f, 594.0f, 4.0f, 5.0f};
    ball_update(&b, 10, 800, 600);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -5.0f, b.vy);
}

void test_speed_is_pythagorean(void) {
    Ball b = {0, 0, 3.0f, 4.0f};
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 5.0f, ball_speed(&b));
}

void test_speed_does_not_change_on_bounce(void) {
    Ball b = {10.0f, 100.0f, -3.0f, 4.0f};
    float speed_before = ball_speed(&b);
    ball_update(&b, 10, 800, 600);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, speed_before, ball_speed(&b));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_normal_movement);
    RUN_TEST(test_left_wall_reflection);
    RUN_TEST(test_right_wall_reflection);
    RUN_TEST(test_top_wall_reflection);
    RUN_TEST(test_bottom_wall_reflection);
    RUN_TEST(test_speed_is_pythagorean);
    RUN_TEST(test_speed_does_not_change_on_bounce);
    return UNITY_END();
}
