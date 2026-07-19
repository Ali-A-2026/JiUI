/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_physics.c
 * @brief Tests for the JiUI Physics engine — bodies, springs, collision,
 *        chain, fluid, gesture momentum, and neural rendering.
 */

#include "jiui/ji_physics.h"
#include "jiui/ji_neural.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

/* =========================================================================
 * Test helpers
 * ========================================================================= */

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST(name) \
    do { \
        printf("  [RUN] %s\n", #name); \
        g_tests_run++; \
        name(); \
        g_tests_passed++; \
        printf("  [PASS] %s\n", #name); \
    } while (0)

#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "    ASSERT FAILED: %s (line %d)\n", #cond, __LINE__); \
            assert(cond); \
        } \
    } while (0)

#define ASSERT_FLOAT_NEAR(a, b, eps) \
    do { \
        if (fabsf((a) - (b)) > (eps)) { \
            fprintf(stderr, "    ASSERT FAILED: %f != %f (line %d)\n", (a), (b), __LINE__); \
            assert(0); \
        } \
    } while (0)

/* =========================================================================
 * Physics World tests
 * ========================================================================= */

static void test_physics_world_lifecycle(void) {
    JiPhysicsWorld* world = ji_physics_world_new();
    ASSERT_TRUE(world != NULL);
    ASSERT_TRUE(ji_physics_world_body_count(world) == 0);
    ji_physics_world_destroy(world);
}

static void test_physics_world_gravity(void) {
    JiPhysicsWorld* world = ji_physics_world_new();
    ji_physics_world_set_gravity(world, 0, 500.0f);

    JiPhysicsBody* body = ji_physics_body_new(100, 100, 1.0f);
    ji_physics_world_add_body(world, body);

    /* Step simulation — body should fall */
    ji_physics_world_step(world, 0.1f);
    float x, y;
    ji_physics_body_position(body, &x, &y);
    ASSERT_TRUE(y > 100.0f); /* Should have fallen */

    ji_physics_body_destroy(body);
    ji_physics_world_destroy(world);
}

static void test_physics_world_bounds(void) {
    JiPhysicsWorld* world = ji_physics_world_new();
    ji_physics_world_set_bounds(world, 0, 0, 200, 200);
    ji_physics_world_set_gravity(world, 0, 1000.0f);

    JiPhysicsBody* body = ji_physics_body_new(100, 190, 1.0f);
    ji_physics_body_set_radius(body, 10.0f);
    ji_physics_body_set_restitution(body, 0.0f); /* No bounce */
    ji_physics_world_add_body(world, body);

    /* Step many times — body should settle at bottom */
    for (int i = 0; i < 100; i++) {
        ji_physics_world_step(world, 0.016f);
    }
    float x, y;
    ji_physics_body_position(body, &x, &y);
    /* Should be at bottom (200 - radius = 190) */
    ASSERT_TRUE(y <= 191.0f);

    ji_physics_body_destroy(body);
    ji_physics_world_destroy(world);
}

static void test_physics_body_add_remove(void) {
    JiPhysicsWorld* world = ji_physics_world_new();
    JiPhysicsBody* b1 = ji_physics_body_new(0, 0, 1.0f);
    JiPhysicsBody* b2 = ji_physics_body_new(50, 50, 2.0f);

    ASSERT_TRUE(ji_physics_world_add_body(world, b1));
    ASSERT_TRUE(ji_physics_world_add_body(world, b2));
    ASSERT_TRUE(ji_physics_world_body_count(world) == 2);

    /* Adding same body again should not duplicate */
    ASSERT_TRUE(ji_physics_world_add_body(world, b1));
    ASSERT_TRUE(ji_physics_world_body_count(world) == 2);

    ASSERT_TRUE(ji_physics_world_remove_body(world, b1));
    ASSERT_TRUE(ji_physics_world_body_count(world) == 1);

    ji_physics_body_destroy(b1);
    ji_physics_body_destroy(b2);
    ji_physics_world_destroy(world);
}

static void test_physics_body_impulse(void) {
    JiPhysicsBody* body = ji_physics_body_new(0, 0, 2.0f);
    ji_physics_body_apply_impulse(body, 10.0f, 0.0f);

    float vx, vy;
    ji_physics_body_velocity(body, &vx, &vy);
    /* Impulse = 10, mass = 2, so velocity = 10/2 = 5 */
    ASSERT_FLOAT_NEAR(vx, 5.0f, 0.01f);
    ASSERT_FLOAT_NEAR(vy, 0.0f, 0.01f);

    ji_physics_body_destroy(body);
}

static void test_physics_body_static(void) {
    JiPhysicsWorld* world = ji_physics_world_new();
    ji_physics_world_set_gravity(world, 0, 1000.0f);

    JiPhysicsBody* body = ji_physics_body_new(100, 100, 1.0f);
    ji_physics_body_set_static(body, true);
    ji_physics_world_add_body(world, body);

    ji_physics_world_step(world, 0.1f);
    float x, y;
    ji_physics_body_position(body, &x, &y);
    /* Static body should not move */
    ASSERT_FLOAT_NEAR(x, 100.0f, 0.01f);
    ASSERT_FLOAT_NEAR(y, 100.0f, 0.01f);

    ji_physics_body_destroy(body);
    ji_physics_world_destroy(world);
}

/* =========================================================================
 * Spring Physics tests
 * ========================================================================= */

static void test_spring_animate(void) {
    float current = 0.0f;
    float velocity = 0.0f;
    float target = 100.0f;

    /* Animate towards target */
    for (int i = 0; i < 100; i++) {
        current = ji_spring_animate(current, &velocity, target, 100.0f, 10.0f, 0.016f);
    }
    /* Should be close to target */
    ASSERT_FLOAT_NEAR(current, target, 5.0f);
}

static void test_spring_overshoot(void) {
    float current = 0.0f;
    float velocity = 0.0f;
    float target = 100.0f;

    /* Underdamped spring should overshoot */
    bool overshot = false;
    for (int i = 0; i < 200; i++) {
        current = ji_spring_animate(current, &velocity, target, 200.0f, 2.0f, 0.016f);
        if (current > target + 0.1f) overshot = true;
    }
    ASSERT_TRUE(overshot);
}

static void test_elastic_ease_out(void) {
    float start = ji_elastic_ease_out(0.0f, 1.0f);
    float end = ji_elastic_ease_out(1.0f, 1.0f);
    float mid = ji_elastic_ease_out(0.5f, 1.0f);

    ASSERT_FLOAT_NEAR(start, 0.0f, 0.01f);
    ASSERT_FLOAT_NEAR(end, 1.0f, 0.01f);
    ASSERT_TRUE(mid > 0.0f && mid < 2.0f); /* May overshoot */
}

/* =========================================================================
 * Gesture Physics tests
 * ========================================================================= */

static void test_gesture_momentum(void) {
    float velocity = 1000.0f;
    bool still_moving = true;

    /* Apply momentum with friction */
    for (int i = 0; i < 200; i++) {
        still_moving = ji_gesture_momentum(&velocity, 5.0f, 0.016f);
        if (!still_moving) break;
    }
    /* Should eventually stop */
    ASSERT_TRUE(!still_moving);
    ASSERT_FLOAT_NEAR(velocity, 0.0f, 0.1f);
}

static void test_gesture_momentum_slow(void) {
    float velocity = 100.0f;
    /* Low friction — should still be moving after a short time */
    ji_gesture_momentum(&velocity, 0.5f, 0.016f);
    ASSERT_TRUE(velocity > 50.0f);
}

/* =========================================================================
 * Collision tests
 * ========================================================================= */

static void test_collision_detect(void) {
    JiPhysicsBody* a = ji_physics_body_new(0, 0, 1.0f);
    JiPhysicsBody* b = ji_physics_body_new(15, 0, 1.0f);
    ji_physics_body_set_radius(a, 10.0f);
    ji_physics_body_set_radius(b, 10.0f);

    /* Distance = 15, sum of radii = 20 → colliding */
    ASSERT_TRUE(ji_physics_bodies_collide(a, b));

    /* Move apart */
    ji_physics_body_set_position(b, 25, 0);
    ASSERT_TRUE(!ji_physics_bodies_collide(a, b));

    ji_physics_body_destroy(a);
    ji_physics_body_destroy(b);
}

static void test_collision_resolve(void) {
    JiPhysicsBody* a = ji_physics_body_new(0, 0, 1.0f);
    JiPhysicsBody* b = ji_physics_body_new(15, 0, 1.0f);
    ji_physics_body_set_radius(a, 10.0f);
    ji_physics_body_set_radius(b, 10.0f);
    ji_physics_body_set_restitution(a, 1.0f);
    ji_physics_body_set_restitution(b, 1.0f);

    /* Give body a velocity towards b */
    ji_physics_body_set_velocity(a, 100, 0);

    ji_physics_resolve_collision(a, b);

    float avx, avy, bvx, bvy;
    ji_physics_body_velocity(a, &avx, &avy);
    ji_physics_body_velocity(b, &bvx, &bvy);

    /* After collision, a should slow down and b should move */
    ASSERT_TRUE(avx < 100.0f);
    ASSERT_TRUE(bvx > 0.0f);

    ji_physics_body_destroy(a);
    ji_physics_body_destroy(b);
}

/* =========================================================================
 * Chain Physics tests
 * ========================================================================= */

static void test_chain_new(void) {
    JiPhysicsWorld* world = ji_physics_world_new();
    JiPhysicsBody** chain = ji_physics_chain_new(world, 100, 50, 5, 20.0f, 1.0f, 100.0f);

    ASSERT_TRUE(chain != NULL);
    ASSERT_TRUE(chain[0] != NULL);
    ASSERT_TRUE(chain[4] != NULL);
    ASSERT_TRUE(chain[5] == NULL); /* NULL-terminated */

    /* First link should be at start position */
    float x, y;
    ji_physics_body_position(chain[0], &x, &y);
    ASSERT_FLOAT_NEAR(x, 100.0f, 0.01f);
    ASSERT_FLOAT_NEAR(y, 50.0f, 0.01f);

    /* Step simulation — chain should fall under gravity */
    for (int i = 0; i < 60; i++) {
        ji_physics_world_step(world, 0.016f);
    }
    ji_physics_body_position(chain[4], &x, &y);
    ASSERT_TRUE(y > 50.0f); /* Last link should have fallen */

    ji_physics_chain_destroy(chain);
    ji_physics_world_destroy(world);
}

/* =========================================================================
 * Fluid tests
 * ========================================================================= */

static void test_fluid_lifecycle(void) {
    JiFluidField* fluid = ji_fluid_new(100, 0.5f, 0.1f);
    ASSERT_TRUE(fluid != NULL);
    ji_fluid_destroy(fluid);
}

static void test_fluid_disturb(void) {
    JiFluidField* fluid = ji_fluid_new(50, 0.5f, 0.01f);

    /* Disturb at center */
    ji_fluid_disturb(fluid, 25, 10.0f);
    ASSERT_FLOAT_NEAR(ji_fluid_height(fluid, 25), 10.0f, 0.01f);

    /* Step — wave should propagate */
    ji_fluid_step(fluid, 0.016f);

    /* Neighbors should have some height now */
    float h_left = ji_fluid_height(fluid, 24);
    float h_right = ji_fluid_height(fluid, 26);
    ASSERT_TRUE(h_left != 0.0f || h_right != 0.0f);

    ji_fluid_destroy(fluid);
}

static void test_fluid_damping(void) {
    JiFluidField* fluid = ji_fluid_new(50, 0.5f, 0.5f); /* High damping */

    ji_fluid_disturb(fluid, 25, 100.0f);

    /* Step many times — should damp out */
    for (int i = 0; i < 200; i++) {
        ji_fluid_step(fluid, 0.016f);
    }

    float h = fabsf(ji_fluid_height(fluid, 25));
    ASSERT_TRUE(h < 10.0f); /* Should be heavily damped */

    ji_fluid_destroy(fluid);
}

/* =========================================================================
 * Neural Model tests
 * ========================================================================= */

static void test_neural_lifecycle(void) {
    JiNeuralModel* model = ji_neural_new();
    ASSERT_TRUE(model != NULL);
    ji_neural_destroy(model);
}

static void test_neural_render_time(void) {
    JiNeuralModel* model = ji_neural_new();

    /* Record some render times */
    for (int i = 0; i < 100; i++) {
        ji_neural_record_render_time(model, 20.0f);
    }
    /* Average should be close to 20ms */
    ASSERT_FLOAT_NEAR(ji_neural_avg_render_time(model), 20.0f, 2.0f);

    ji_neural_destroy(model);
}

static void test_neural_layout_suggestion(void) {
    JiNeuralModel* model = ji_neural_new();

    /* Record slow render times */
    for (int i = 0; i < 100; i++) {
        ji_neural_record_render_time(model, 40.0f); /* 25fps */
    }

    /* Target 60fps (16.67ms) — should suggest complexity reduction */
    float complexity = ji_neural_suggest_layout_complexity(model, 60.0f);
    ASSERT_TRUE(complexity < 1.0f); /* Should suggest reduction */

    ji_neural_destroy(model);
}

static void test_neural_interaction_prediction(void) {
    JiNeuralModel* model = ji_neural_new();

    /* Train: interaction 1 → 2 → 3 → 2 → 3 (pattern) */
    ji_neural_record_interaction(model, 1);
    ji_neural_record_interaction(model, 2);
    ji_neural_record_interaction(model, 3);
    ji_neural_record_interaction(model, 2);
    ji_neural_record_interaction(model, 3);
    ji_neural_record_interaction(model, 2);

    /* After 2, most likely next is 3 */
    int predicted = ji_neural_predict_next_interaction(model, 2);
    ASSERT_TRUE(predicted == 3);

    /* Probability of 2→3 should be high */
    float prob = ji_neural_interaction_probability(model, 2, 3);
    ASSERT_TRUE(prob > 0.5f);

    ji_neural_destroy(model);
}

static void test_neural_animation_timing(void) {
    JiNeuralModel* model = ji_neural_new();

    /* Record preferred durations */
    for (int i = 0; i < 50; i++) {
        ji_neural_record_animation(model, 200.0f);
    }

    float suggested = ji_neural_suggested_duration(model);
    ASSERT_FLOAT_NEAR(suggested, 200.0f, 30.0f);

    ji_neural_destroy(model);
}

static void test_neural_accessibility_font(void) {
    JiNeuralModel* model = ji_neural_new();

    /* Slow reader → larger font */
    for (int i = 0; i < 50; i++) {
        ji_neural_record_reading_speed(model, 80.0f);
    }
    ASSERT_TRUE(ji_neural_auto_font_scale(model) > 1.0f);

    /* Fast reader → normal or smaller font */
    JiNeuralModel* model2 = ji_neural_new();
    for (int i = 0; i < 50; i++) {
        ji_neural_record_reading_speed(model2, 400.0f);
    }
    ASSERT_TRUE(ji_neural_auto_font_scale(model2) <= 1.0f);

    ji_neural_destroy(model);
    ji_neural_destroy(model2);
}

static void test_neural_contrast_ratio(void) {
    /* Black on white → ratio should be 21 */
    float ratio = ji_neural_contrast_ratio(0, 0, 0, 1, 1, 1);
    ASSERT_FLOAT_NEAR(ratio, 21.0f, 0.5f);

    /* White on white → ratio should be 1 */
    ratio = ji_neural_contrast_ratio(1, 1, 1, 1, 1, 1);
    ASSERT_FLOAT_NEAR(ratio, 1.0f, 0.01f);
}

static void test_neural_suggest_fg_color(void) {
    float r, g, b;

    /* Dark background → suggest white */
    ji_neural_suggest_fg_color(0.1f, 0.1f, 0.1f, &r, &g, &b);
    ASSERT_FLOAT_NEAR(r, 1.0f, 0.01f);
    ASSERT_FLOAT_NEAR(g, 1.0f, 0.01f);
    ASSERT_FLOAT_NEAR(b, 1.0f, 0.01f);

    /* Light background → suggest black */
    ji_neural_suggest_fg_color(0.9f, 0.9f, 0.9f, &r, &g, &b);
    ASSERT_FLOAT_NEAR(r, 0.0f, 0.01f);
    ASSERT_FLOAT_NEAR(g, 0.0f, 0.01f);
    ASSERT_FLOAT_NEAR(b, 0.0f, 0.01f);
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void) {
    printf("=== JiUI Physics & Neural Tests ===\n\n");

    printf("--- Physics World ---\n");
    TEST(test_physics_world_lifecycle);
    TEST(test_physics_world_gravity);
    TEST(test_physics_world_bounds);
    TEST(test_physics_body_add_remove);
    TEST(test_physics_body_impulse);
    TEST(test_physics_body_static);

    printf("\n--- Spring Physics ---\n");
    TEST(test_spring_animate);
    TEST(test_spring_overshoot);
    TEST(test_elastic_ease_out);

    printf("\n--- Gesture Physics ---\n");
    TEST(test_gesture_momentum);
    TEST(test_gesture_momentum_slow);

    printf("\n--- Collision ---\n");
    TEST(test_collision_detect);
    TEST(test_collision_resolve);

    printf("\n--- Chain Physics ---\n");
    TEST(test_chain_new);

    printf("\n--- Fluid ---\n");
    TEST(test_fluid_lifecycle);
    TEST(test_fluid_disturb);
    TEST(test_fluid_damping);

    printf("\n--- Neural Model ---\n");
    TEST(test_neural_lifecycle);
    TEST(test_neural_render_time);
    TEST(test_neural_layout_suggestion);
    TEST(test_neural_interaction_prediction);
    TEST(test_neural_animation_timing);
    TEST(test_neural_accessibility_font);
    TEST(test_neural_contrast_ratio);
    TEST(test_neural_suggest_fg_color);

    printf("\n=== Results: %d/%d passed ===\n", g_tests_passed, g_tests_run);
    return (g_tests_passed == g_tests_run) ? 0 : 1;
}
