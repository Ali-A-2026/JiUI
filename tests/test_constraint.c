/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file test_constraint.c
 * @brief Tests for the Cassowary constraint solver.
 */

#include <jiui/jiui.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { tests_run++; printf("  [RUN] %s\n", #name); name(); tests_passed++; \
         printf("  [PASS] %s\n", #name); } while(0)

#define ASSERT_TRUE(cond) \
    do { if (!(cond)) { printf("  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, #cond); assert(cond); } } while(0)

#define ASSERT_NEAR(a, b, eps) \
    do { if (fabs((a) - (b)) > (eps)) { printf("  [FAIL] %s:%d: %g != %g (eps %g)\n", \
        __FILE__, __LINE__, (double)(a), (double)(b), (double)(eps)); assert(0); } } while(0)

static void test_variable_create(void) {
    JiVariable* v = ji_variable_new(42.0);
    ASSERT_TRUE(v != NULL);
    ASSERT_NEAR(ji_variable_get_value(v), 42.0, 1e-6);
    ji_variable_set_value(v, 100.0);
    ASSERT_NEAR(ji_variable_get_value(v), 100.0, 1e-6);
    ji_variable_set_name(v, "test_var");
    ASSERT_TRUE(strcmp(ji_variable_get_name(v), "test_var") == 0);
    ji_variable_destroy(v);
}

static void test_constraint_create(void) {
    JiVariable* a = ji_variable_new(10.0);
    JiVariable* b = ji_variable_new(20.0);

    JiConstraint* c = ji_constraint_new(a, JI_REL_EQ, b, 1.0, 5.0, JI_STRENGTH_REQUIRED);
    ASSERT_TRUE(c != NULL);
    ASSERT_TRUE(ji_constraint_get_strength(c) == JI_STRENGTH_REQUIRED);
    ASSERT_TRUE(ji_constraint_get_relation(c) == JI_REL_EQ);

    ji_constraint_set_strength(c, JI_STRENGTH_STRONG);
    ASSERT_TRUE(ji_constraint_get_strength(c) == JI_STRENGTH_STRONG);

    ji_constraint_destroy(c);
    ji_variable_destroy(a);
    ji_variable_destroy(b);
}

static void test_solver_basic(void) {
    JiConstraintSolver* s = ji_constraint_solver_new();
    ASSERT_TRUE(s != NULL);

    JiVariable* x = ji_variable_new(0.0);
    JiVariable* y = ji_variable_new(0.0);

    /* x == y + 10 */
    JiConstraint* c = ji_constraint_new(x, JI_REL_EQ, y, 1.0, 10.0, JI_STRENGTH_REQUIRED);
    ji_constraint_solver_add_constraint(s, c);

    /* y == 50 */
    JiConstraint* c2 = ji_constraint_new_const(y, JI_REL_EQ, 50.0, JI_STRENGTH_REQUIRED);
    ji_constraint_solver_add_constraint(s, c2);

    bool ok = ji_constraint_solver_solve(s);
    ASSERT_TRUE(ok);
    ASSERT_NEAR(ji_variable_get_value(y), 50.0, 1e-4);
    ASSERT_NEAR(ji_variable_get_value(x), 60.0, 1e-4);

    ji_constraint_solver_destroy(s);
    ji_constraint_destroy(c);
    ji_constraint_destroy(c2);
    ji_variable_destroy(x);
    ji_variable_destroy(y);
}

static void test_solver_stay(void) {
    JiConstraintSolver* s = ji_constraint_solver_new();

    JiVariable* x = ji_variable_new(100.0);
    ji_constraint_solver_add_stay(s, x, JI_STRENGTH_STRONG);

    bool ok = ji_constraint_solver_solve(s);
    ASSERT_TRUE(ok);
    ASSERT_NEAR(ji_variable_get_value(x), 100.0, 1e-4);

    ji_constraint_solver_destroy(s);
    ji_variable_destroy(x);
}

static void test_solver_edit_var(void) {
    JiConstraintSolver* s = ji_constraint_solver_new();

    JiVariable* x = ji_variable_new(0.0);
    JiVariable* y = ji_variable_new(0.0);

    /* x == y + 10 */
    JiConstraint* c = ji_constraint_new(x, JI_REL_EQ, y, 1.0, 10.0, JI_STRENGTH_REQUIRED);
    ji_constraint_solver_add_constraint(s, c);

    ji_constraint_solver_add_edit_var(s, y, JI_STRENGTH_REQUIRED);
    ji_constraint_solver_suggest_value(s, y, 200.0);

    bool ok = ji_constraint_solver_solve(s);
    ASSERT_TRUE(ok);
    ASSERT_NEAR(ji_variable_get_value(y), 200.0, 1e-4);
    ASSERT_NEAR(ji_variable_get_value(x), 210.0, 1e-4);

    ji_constraint_solver_destroy(s);
    ji_constraint_destroy(c);
    ji_variable_destroy(x);
    ji_variable_destroy(y);
}

static void test_solver_inequality(void) {
    JiConstraintSolver* s = ji_constraint_solver_new();

    JiVariable* x = ji_variable_new(0.0);

    /* x >= 50 */
    JiConstraint* c = ji_constraint_new_const(x, JI_REL_GE, 50.0, JI_STRENGTH_REQUIRED);
    ji_constraint_solver_add_constraint(s, c);

    /* x <= 100 */
    JiConstraint* c2 = ji_constraint_new_const(x, JI_REL_LE, 100.0, JI_STRENGTH_REQUIRED);
    ji_constraint_solver_add_constraint(s, c2);

    /* Stay at 75 */
    ji_constraint_solver_add_stay(s, x, JI_STRENGTH_WEAK);
    ji_variable_set_value(x, 75.0);

    bool ok = ji_constraint_solver_solve(s);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(ji_variable_get_value(x) >= 50.0 - 1e-4);
    ASSERT_TRUE(ji_variable_get_value(x) <= 100.0 + 1e-4);

    ji_constraint_solver_destroy(s);
    ji_constraint_destroy(c);
    ji_constraint_destroy(c2);
    ji_variable_destroy(x);
}

static void test_solver_count(void) {
    JiConstraintSolver* s = ji_constraint_solver_new();
    ASSERT_TRUE(ji_constraint_solver_constraint_count(s) == 0);
    ASSERT_TRUE(ji_constraint_solver_variable_count(s) == 0);

    JiVariable* x = ji_variable_new(0.0);
    JiVariable* y = ji_variable_new(0.0);
    JiConstraint* c = ji_constraint_new(x, JI_REL_EQ, y, 1.0, 0.0, JI_STRENGTH_REQUIRED);
    ji_constraint_solver_add_constraint(s, c);

    ASSERT_TRUE(ji_constraint_solver_constraint_count(s) == 1);
    ASSERT_TRUE(ji_constraint_solver_variable_count(s) == 2);

    ji_constraint_solver_remove_constraint(s, c);
    ASSERT_TRUE(ji_constraint_solver_constraint_count(s) == 0);

    ji_constraint_solver_destroy(s);
    ji_constraint_destroy(c);
    ji_variable_destroy(x);
    ji_variable_destroy(y);
}

static void test_anchor_helpers(void) {
    JiVariable* left = ji_variable_new(0.0);
    JiVariable* right = ji_variable_new(0.0);
    JiVariable* anchor = ji_variable_new(100.0);

    JiConstraint* c = ji_constraint_anchor_left(left, anchor, 10.0, JI_STRENGTH_REQUIRED);
    ASSERT_TRUE(c != NULL);

    JiConstraint* w = ji_constraint_width(left, right, 200.0, JI_STRENGTH_REQUIRED);
    ASSERT_TRUE(w != NULL);

    JiConstraintSolver* s = ji_constraint_solver_new();
    ji_constraint_solver_add_constraint(s, c);
    ji_constraint_solver_add_constraint(s, w);
    ji_constraint_solver_add_stay(s, anchor, JI_STRENGTH_REQUIRED);

    bool ok = ji_constraint_solver_solve(s);
    ASSERT_TRUE(ok);
    ASSERT_NEAR(ji_variable_get_value(left), 110.0, 1e-4);
    ASSERT_NEAR(ji_variable_get_value(right), 310.0, 1e-4);

    ji_constraint_solver_destroy(s);
    ji_constraint_destroy(c);
    ji_constraint_destroy(w);
    ji_variable_destroy(left);
    ji_variable_destroy(right);
    ji_variable_destroy(anchor);
}

int main(void) {
    printf("=== Constraint Solver Tests ===\n");
    TEST(test_variable_create);
    TEST(test_constraint_create);
    TEST(test_solver_basic);
    TEST(test_solver_stay);
    TEST(test_solver_edit_var);
    TEST(test_solver_inequality);
    TEST(test_solver_count);
    TEST(test_anchor_helpers);
    printf("=== %d/%d tests passed ===\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
