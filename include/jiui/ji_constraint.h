/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file ji_constraint.h
 * @brief Constraint-based layout solver — Cassowary algorithm for adaptive UI.
 *
 * Implements the Cassowary constraint solving algorithm, similar to macOS
 * Auto Layout. Supports linear constraints with priorities (required, strong,
 * medium, weak), stay constraints, and edit variables for interactive
 * dragging/resizing.
 */

#ifndef JIUI_CONSTRAINT_H
#define JIUI_CONSTRAINT_H

#include "ji_types.h"
#include "ji_api.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Constraint Types
 * ========================================================================= */

typedef enum JiRelation {
    JI_REL_LE,   /**< <= */
    JI_REL_GE,   /**< >= */
    JI_REL_EQ    /**< == */
} JiRelation;

typedef enum JiStrength {
    JI_STRENGTH_REQUIRED = 1000000000,
    JI_STRENGTH_STRONG   = 1000000,
    JI_STRENGTH_MEDIUM   = 1000,
    JI_STRENGTH_WEAK     = 1
} JiStrength;

/* =========================================================================
 * Variable
 * ========================================================================= */

typedef struct JiVariable JiVariable;

JI_API JiVariable* ji_variable_new(double value);
JI_API void        ji_variable_destroy(JiVariable* var);

JI_API double      ji_variable_get_value(const JiVariable* var);
JI_API void        ji_variable_set_value(JiVariable* var, double value);

JI_API const char* ji_variable_get_name(const JiVariable* var);
JI_API void        ji_variable_set_name(JiVariable* var, const char* name);

/* =========================================================================
 * Constraint
 * ========================================================================= */

typedef struct JiConstraint JiConstraint;

/** Create a simple constraint:  var1 op var2 * multiplier + constant  */
JI_API JiConstraint* ji_constraint_new(JiVariable* var1, JiRelation op,
                                 JiVariable* var2, double multiplier,
                                 double constant, double strength);

/** Create a constant constraint:  var op constant  */
JI_API JiConstraint* ji_constraint_new_const(JiVariable* var, JiRelation op,
                                       double constant, double strength);

JI_API void           ji_constraint_destroy(JiConstraint* constraint);

JI_API double         ji_constraint_get_strength(const JiConstraint* c);
JI_API void           ji_constraint_set_strength(JiConstraint* c, double strength);

JI_API JiRelation     ji_constraint_get_relation(const JiConstraint* c);

/* =========================================================================
 * Constraint Solver (Cassowary)
 * ========================================================================= */

typedef struct JiConstraintSolver JiConstraintSolver;

JI_API JiConstraintSolver* ji_constraint_solver_new(void);
JI_API void                ji_constraint_solver_destroy(JiConstraintSolver* solver);

/** Add a constraint to the solver. Returns false if the constraint is
 *  unsatisfiable at required strength. */
JI_API bool ji_constraint_solver_add_constraint(JiConstraintSolver* solver,
                                          JiConstraint* constraint);

/** Remove a previously added constraint. */
JI_API bool ji_constraint_solver_remove_constraint(JiConstraintSolver* solver,
                                              JiConstraint* constraint);

/** Add a stay constraint (keep variable at current value). */
JI_API bool ji_constraint_solver_add_stay(JiConstraintSolver* solver,
                                     JiVariable* var, double strength);

/** Remove a stay constraint. */
JI_API bool ji_constraint_solver_remove_stay(JiConstraintSolver* solver,
                                        JiVariable* var);

/** Add an edit variable (suggest a new value during solving). */
JI_API bool ji_constraint_solver_add_edit_var(JiConstraintSolver* solver,
                                          JiVariable* var, double strength);

/** Remove an edit variable. */
JI_API bool ji_constraint_solver_remove_edit_var(JiConstraintSolver* solver,
                                            JiVariable* var);

/** Suggest a new value for an edit variable. */
JI_API bool ji_constraint_solver_suggest_value(JiConstraintSolver* solver,
                                          JiVariable* var, double value);

/** Solve all constraints. Returns false if no solution exists. */
JI_API bool ji_constraint_solver_solve(JiConstraintSolver* solver);

/** Reset the solver to initial state. */
JI_API void ji_constraint_solver_reset(JiConstraintSolver* solver);

/** Get the number of constraints. */
JI_API uint32_t ji_constraint_solver_constraint_count(const JiConstraintSolver* solver);

/** Get the number of variables. */
JI_API uint32_t ji_constraint_solver_variable_count(const JiConstraintSolver* solver);

/* =========================================================================
 * Convenience: Widget Constraint Helpers
 * ========================================================================= */

/** Constrain widget.left == anchor + offset at given strength. */
JI_API JiConstraint* ji_constraint_anchor_left(JiVariable* widget_left,
                                         JiVariable* anchor, double offset,
                                         double strength);

/** Constrain widget.right == anchor + offset. */
JI_API JiConstraint* ji_constraint_anchor_right(JiVariable* widget_right,
                                          JiVariable* anchor, double offset,
                                          double strength);

/** Constrain widget.top == anchor + offset. */
JI_API JiConstraint* ji_constraint_anchor_top(JiVariable* widget_top,
                                        JiVariable* anchor, double offset,
                                        double strength);

/** Constrain widget.bottom == anchor + offset. */
JI_API JiConstraint* ji_constraint_anchor_bottom(JiVariable* widget_bottom,
                                           JiVariable* anchor, double offset,
                                           double strength);

/** Constrain widget width == value. */
JI_API JiConstraint* ji_constraint_width(JiVariable* left, JiVariable* right,
                                   double width, double strength);

/** Constrain widget height == value. */
JI_API JiConstraint* ji_constraint_height(JiVariable* top, JiVariable* bottom,
                                    double height, double strength);

/** Constrain widget center X == anchor. */
JI_API JiConstraint* ji_constraint_center_x(JiVariable* left, JiVariable* right,
                                       JiVariable* anchor, double strength);

/** Constrain widget center Y == anchor. */
JI_API JiConstraint* ji_constraint_center_y(JiVariable* top, JiVariable* bottom,
                                        JiVariable* anchor, double strength);

/** Constrain aspect ratio: width / height == ratio. */
JI_API JiConstraint* ji_constraint_aspect_ratio(JiVariable* left, JiVariable* right,
                                           JiVariable* top, JiVariable* bottom,
                                           double ratio, double strength);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_CONSTRAINT_H */
