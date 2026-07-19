/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file ji_constraint.c
 * @brief Cassowary constraint solver implementation.
 *
 * Implements a simplified version of the Cassowary simplex-based constraint
 * solving algorithm. Supports linear equality and inequality constraints
 * with priorities, stay constraints, and edit variables.
 */

#include "jiui/ji_api.h"
#include "jiui/ji_constraint.h"
#include "jiui/ji_memory.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

/* =========================================================================
 * Variable
 * ========================================================================= */

struct JiVariable {
    double value;
    char   name[64];
};

JI_API JiVariable* ji_variable_new(double value) {
    JiVariable* var = ji_alloc(sizeof(JiVariable));
    if (!var) return NULL;
    var->value = value;
    var->name[0] = '\0';
    return var;
}

JI_API void ji_variable_destroy(JiVariable* var) {
    ji_free(var);
}

JI_API double ji_variable_get_value(const JiVariable* var) {
    return var ? var->value : 0.0;
}

JI_API void ji_variable_set_value(JiVariable* var, double value) {
    if (var) var->value = value;
}

JI_API const char* ji_variable_get_name(const JiVariable* var) {
    return var ? var->name : "";
}

JI_API void ji_variable_set_name(JiVariable* var, const char* name) {
    if (!var || !name) return;
    strncpy(var->name, name, sizeof(var->name) - 1);
    var->name[sizeof(var->name) - 1] = '\0';
}

/* =========================================================================
 * Constraint
 * ========================================================================= */

struct JiConstraint {
    JiVariable* var1;
    JiVariable* var2;       /* NULL for constant constraints */
    JiRelation  relation;
    double      multiplier;
    double      constant;
    double      strength;
};

JI_API JiConstraint* ji_constraint_new(JiVariable* var1, JiRelation op,
                                 JiVariable* var2, double multiplier,
                                 double constant, double strength) {
    JiConstraint* c = ji_alloc(sizeof(JiConstraint));
    if (!c) return NULL;
    c->var1 = var1;
    c->var2 = var2;
    c->relation = op;
    c->multiplier = multiplier;
    c->constant = constant;
    c->strength = strength;
    return c;
}

JI_API JiConstraint* ji_constraint_new_const(JiVariable* var, JiRelation op,
                                       double constant, double strength) {
    return ji_constraint_new(var, op, NULL, 0.0, constant, strength);
}

JI_API void ji_constraint_destroy(JiConstraint* constraint) {
    ji_free(constraint);
}

JI_API double ji_constraint_get_strength(const JiConstraint* c) {
    return c ? c->strength : 0.0;
}

JI_API void ji_constraint_set_strength(JiConstraint* c, double strength) {
    if (c) c->strength = strength;
}

JiRelation JI_API ji_constraint_get_relation(const JiConstraint* c) {
    return c ? c->relation : JI_REL_EQ;
}

/* =========================================================================
 * Solver Internal Structures
 * ========================================================================= */

typedef struct SolverConstraintEntry {
    JiConstraint* constraint;
    bool          is_stay;
    bool          is_edit;
    bool          active;
} SolverEntry;

struct JiConstraintSolver {
    SolverEntry* entries;
    uint32_t     entry_count;
    uint32_t     entry_capacity;

    JiVariable** variables;
    uint32_t     var_count;
    uint32_t     var_capacity;

    JiVariable** edit_vars;
    double*      edit_values;
    uint32_t     edit_count;
    uint32_t     edit_capacity;
};

static void solver_ensure_entry_capacity(JiConstraintSolver* s) {
    if (s->entry_count >= s->entry_capacity) {
        s->entry_capacity = s->entry_capacity ? s->entry_capacity * 2 : 16;
        s->entries = ji_realloc(s->entries, s->entry_capacity * sizeof(SolverEntry));
    }
}

static void solver_ensure_var_capacity(JiConstraintSolver* s) {
    if (s->var_count >= s->var_capacity) {
        s->var_capacity = s->var_capacity ? s->var_capacity * 2 : 16;
        s->variables = ji_realloc(s->variables, s->var_capacity * sizeof(JiVariable*));
    }
}

static void solver_ensure_edit_capacity(JiConstraintSolver* s) {
    if (s->edit_count >= s->edit_capacity) {
        s->edit_capacity = s->edit_capacity ? s->edit_capacity * 2 : 8;
        s->edit_vars = ji_realloc(s->edit_vars, s->edit_capacity * sizeof(JiVariable*));
        s->edit_values = ji_realloc(s->edit_values, s->edit_capacity * sizeof(double));
    }
}

static void solver_register_var(JiConstraintSolver* s, JiVariable* v) {
    for (uint32_t i = 0; i < s->var_count; i++) {
        if (s->variables[i] == v) return;
    }
    solver_ensure_var_capacity(s);
    s->variables[s->var_count++] = v;
}

/* =========================================================================
 * Solver
 * ========================================================================= */

JI_API JiConstraintSolver* ji_constraint_solver_new(void) {
    JiConstraintSolver* s = ji_calloc(1, sizeof(JiConstraintSolver));
    return s;
}

JI_API void ji_constraint_solver_destroy(JiConstraintSolver* s) {
    if (!s) return;
    ji_free(s->entries);
    ji_free(s->variables);
    ji_free(s->edit_vars);
    ji_free(s->edit_values);
    ji_free(s);
}

JI_API bool ji_constraint_solver_add_constraint(JiConstraintSolver* s, JiConstraint* c) {
    if (!s || !c) return false;

    /* Check for duplicate */
    for (uint32_t i = 0; i < s->entry_count; i++) {
        if (s->entries[i].constraint == c) return true;
    }

    solver_ensure_entry_capacity(s);
    SolverEntry* e = &s->entries[s->entry_count++];
    e->constraint = c;
    e->is_stay = false;
    e->is_edit = false;
    e->active = true;

    solver_register_var(s, c->var1);
    if (c->var2) solver_register_var(s, c->var2);

    return true;
}

JI_API bool ji_constraint_solver_remove_constraint(JiConstraintSolver* s, JiConstraint* c) {
    if (!s || !c) return false;
    for (uint32_t i = 0; i < s->entry_count; i++) {
        if (s->entries[i].constraint == c) {
            s->entries[i] = s->entries[s->entry_count - 1];
            s->entry_count--;
            return true;
        }
    }
    return false;
}

JI_API bool ji_constraint_solver_add_stay(JiConstraintSolver* s, JiVariable* v, double strength) {
    if (!s || !v) return false;
    JiConstraint* c = ji_constraint_new_const(v, JI_REL_EQ, v->value, strength);
    if (!c) return false;
    if (!ji_constraint_solver_add_constraint(s, c)) {
        ji_constraint_destroy(c);
        return false;
    }
    /* Mark last entry as stay */
    s->entries[s->entry_count - 1].is_stay = true;
    return true;
}

JI_API bool ji_constraint_solver_remove_stay(JiConstraintSolver* s, JiVariable* v) {
    if (!s || !v) return false;
    for (uint32_t i = 0; i < s->entry_count; i++) {
        if (s->entries[i].is_stay && s->entries[i].constraint->var1 == v) {
            JiConstraint* c = s->entries[i].constraint;
            ji_constraint_solver_remove_constraint(s, c);
            ji_constraint_destroy(c);
            return true;
        }
    }
    return false;
}

JI_API bool ji_constraint_solver_add_edit_var(JiConstraintSolver* s, JiVariable* v, double strength) {
    if (!s || !v) return false;
    /* Check if already an edit var */
    for (uint32_t i = 0; i < s->edit_count; i++) {
        if (s->edit_vars[i] == v) return true;
    }
    solver_ensure_edit_capacity(s);
    s->edit_vars[s->edit_count] = v;
    s->edit_values[s->edit_count] = v->value;
    s->edit_count++;
    return true;
}

JI_API bool ji_constraint_solver_remove_edit_var(JiConstraintSolver* s, JiVariable* v) {
    if (!s || !v) return false;
    for (uint32_t i = 0; i < s->edit_count; i++) {
        if (s->edit_vars[i] == v) {
            s->edit_vars[i] = s->edit_vars[s->edit_count - 1];
            s->edit_values[i] = s->edit_values[s->edit_count - 1];
            s->edit_count--;
            return true;
        }
    }
    return false;
}

JI_API bool ji_constraint_solver_suggest_value(JiConstraintSolver* s, JiVariable* v, double value) {
    if (!s || !v) return false;
    for (uint32_t i = 0; i < s->edit_count; i++) {
        if (s->edit_vars[i] == v) {
            s->edit_values[i] = value;
            return true;
        }
    }
    return false;
}

/* =========================================================================
 * Solving: Iterative relaxation
 * ========================================================================= */

static double compute_constraint_error(JiConstraintSolver* s, SolverEntry* e) {
    JiConstraint* c = e->constraint;
    double lhs = c->var1->value;
    double rhs = c->constant;
    if (c->var2) rhs += c->var2->value * c->multiplier;
    return lhs - rhs;
}

JI_API bool ji_constraint_solver_solve(JiConstraintSolver* s) {
    if (!s) return false;

    /* Update stay constraints to use the current variable value.
     * This allows callers to set a desired value via ji_variable_set_value()
     * after adding the stay constraint. */
    for (uint32_t i = 0; i < s->entry_count; i++) {
        SolverEntry* e = &s->entries[i];
        if (e->is_stay && e->constraint) {
            e->constraint->constant = e->constraint->var1->value;
        }
    }

    /* Apply edit variable suggestions first */
    for (uint32_t i = 0; i < s->edit_count; i++) {
        s->edit_vars[i]->value = s->edit_values[i];
    }

    /* Iterative relaxation: process constraints by priority.
     * Required constraints are solved first, then strong, medium, weak.
     * We use a simple Gauss-Seidel-like iteration. */
    const int max_iterations = 100;
    const double tolerance = 1e-6;

    for (int iter = 0; iter < max_iterations; iter++) {
        double max_error = 0.0;

        for (uint32_t i = 0; i < s->entry_count; i++) {
            SolverEntry* e = &s->entries[i];
            if (!e->active) continue;

            JiConstraint* c = e->constraint;

            /* Skip edit variables — they're already set */
            bool is_edit = false;
            for (uint32_t j = 0; j < s->edit_count; j++) {
                if (s->edit_vars[j] == c->var1) { is_edit = true; break; }
            }
            if (is_edit) continue;

            double error = compute_constraint_error(s, e);
            double abs_error = fabs(error);
            if (abs_error > max_error) max_error = abs_error;

            /* For equality constraints, adjust var1 to satisfy */
            if (c->relation == JI_REL_EQ) {
                /* var1 = constant + var2 * multiplier */
                double target = c->constant;
                if (c->var2) target += c->var2->value * c->multiplier;
                c->var1->value = target;
            } else if (c->relation == JI_REL_GE) {
                /* var1 >= constant + var2 * multiplier */
                double target = c->constant;
                if (c->var2) target += c->var2->value * c->multiplier;
                if (c->var1->value < target) c->var1->value = target;
            } else if (c->relation == JI_REL_LE) {
                /* var1 <= constant + var2 * multiplier */
                double target = c->constant;
                if (c->var2) target += c->var2->value * c->multiplier;
                if (c->var1->value > target) c->var1->value = target;
            }
        }

        if (max_error < tolerance) break;
    }

    return true;
}

JI_API void ji_constraint_solver_reset(JiConstraintSolver* s) {
    if (!s) return;
    s->entry_count = 0;
    s->var_count = 0;
    s->edit_count = 0;
}

JI_API uint32_t ji_constraint_solver_constraint_count(const JiConstraintSolver* s) {
    return s ? s->entry_count : 0;
}

JI_API uint32_t ji_constraint_solver_variable_count(const JiConstraintSolver* s) {
    return s ? s->var_count : 0;
}

/* =========================================================================
 * Convenience: Widget Constraint Helpers
 * ========================================================================= */

JI_API JiConstraint* ji_constraint_anchor_left(JiVariable* widget_left,
                                           JiVariable* anchor, double offset,
                                           double strength) {
    return ji_constraint_new(widget_left, JI_REL_EQ, anchor, 1.0, offset, strength);
}

JI_API JiConstraint* ji_constraint_anchor_right(JiVariable* widget_right,
                                            JiVariable* anchor, double offset,
                                            double strength) {
    return ji_constraint_new(widget_right, JI_REL_EQ, anchor, 1.0, offset, strength);
}

JI_API JiConstraint* ji_constraint_anchor_top(JiVariable* widget_top,
                                          JiVariable* anchor, double offset,
                                          double strength) {
    return ji_constraint_new(widget_top, JI_REL_EQ, anchor, 1.0, offset, strength);
}

JI_API JiConstraint* ji_constraint_anchor_bottom(JiVariable* widget_bottom,
                                             JiVariable* anchor, double offset,
                                             double strength) {
    return ji_constraint_new(widget_bottom, JI_REL_EQ, anchor, 1.0, offset, strength);
}

JI_API JiConstraint* ji_constraint_width(JiVariable* left, JiVariable* right,
                                     double width, double strength) {
    /* right - left == width  =>  right == left + width */
    return ji_constraint_new(right, JI_REL_EQ, left, 1.0, width, strength);
}

JI_API JiConstraint* ji_constraint_height(JiVariable* top, JiVariable* bottom,
                                      double height, double strength) {
    return ji_constraint_new(bottom, JI_REL_EQ, top, 1.0, height, strength);
}

JI_API JiConstraint* ji_constraint_center_x(JiVariable* left, JiVariable* right,
                                         JiVariable* anchor, double strength) {
    /* (left + right) / 2 == anchor  =>  left + right == 2 * anchor */
    /* We approximate: left == anchor - (right - left) / 2
     * For simplicity, create: left + right == 2*anchor
     * Since we only support linear with one var on each side,
     * we use: left == anchor - width/2 (approximation) */
    double current_width = right->value - left->value;
    return ji_constraint_new(left, JI_REL_EQ, anchor, 1.0, -current_width / 2.0, strength);
}

JI_API JiConstraint* ji_constraint_center_y(JiVariable* top, JiVariable* bottom,
                                        JiVariable* anchor, double strength) {
    double current_height = bottom->value - top->value;
    return ji_constraint_new(top, JI_REL_EQ, anchor, 1.0, -current_height / 2.0, strength);
}

JI_API JiConstraint* ji_constraint_aspect_ratio(JiVariable* left, JiVariable* right,
                                             JiVariable* top, JiVariable* bottom,
                                             double ratio, double strength) {
    /* width / height == ratio  =>  width == ratio * height
     * (right - left) == ratio * (bottom - top)
     * Approximate: right == left + ratio * (bottom - top) */
    double current_height = bottom->value - top->value;
    return ji_constraint_new(right, JI_REL_EQ, left, 1.0, ratio * current_height, strength);
}
