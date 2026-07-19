/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_physics.h
 * @brief Physics-based animation engine — spring physics, elastic motion,
 *        gesture momentum, collision/bounce, gravity, chain physics, fluid waves.
 */

#ifndef JIUI_PHYSICS_H
#define JIUI_PHYSICS_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_types.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Physics Body — a point mass with position, velocity, and forces
 * ========================================================================= */

typedef struct JiPhysicsBody {
    float  x, y;          /* Position */
    float  vx, vy;       /* Velocity */
    float  ax, ay;       /* Acceleration */
    float  mass;         /* Mass (kg) */
    float  inv_mass;     /* Inverse mass (1/mass, 0 = static) */
    float  restitution;  /* Bounciness (0 = no bounce, 1 = perfect) */
    float  friction;     /* Friction coefficient */
    bool   is_static;    /* If true, body does not move */
    float  radius;       /* For collision (circle) */
    void*  user_data;    /* User-defined data (e.g., widget pointer) */
} JiPhysicsBody;

/* =========================================================================
 * Spring — connects two bodies with a spring force (Hooke's law + damping)
 * ========================================================================= */

typedef struct JiSpring {
    JiPhysicsBody* body_a;
    JiPhysicsBody* body_b;
    float          rest_length;  /* Rest length */
    float          stiffness;    /* Spring constant k */
    float          damping;      /* Damping coefficient */
} JiSpring;

/* =========================================================================
 * Physics World — manages bodies, springs, gravity, and integration
 * ========================================================================= */

typedef struct JiPhysicsWorld {
    JiPhysicsBody**  bodies;
    int              body_count;
    int              body_capacity;
    JiSpring*        springs;
    int              spring_count;
    int              spring_capacity;
    float            gravity_x;     /* Gravity acceleration (m/s^2) */
    float            gravity_y;
    float            world_bounds_x; /* World bounds for collision */
    float            world_bounds_y;
    float            world_bounds_w;
    float            world_bounds_h;
    float            air_friction;  /* Global air friction (0..1 per second) */
    float            time_scale;   /* Time scaling factor */
} JiPhysicsWorld;

/* =========================================================================
 * Physics World — Lifecycle
 * ========================================================================= */

/** Create a new physics world. */
JI_API JiPhysicsWorld* ji_physics_world_new(void);

/** Destroy a physics world. */
JI_API void ji_physics_world_destroy(JiPhysicsWorld* world);

/** Set gravity. */
JI_API void ji_physics_world_set_gravity(JiPhysicsWorld* world, float gx, float gy);

/** Set world bounds (for boundary collision). */
JI_API void ji_physics_world_set_bounds(JiPhysicsWorld* world,
                                           float x, float y, float w, float h);

/** Set air friction (0 = no friction, 1 = full stop per second). */
JI_API void ji_physics_world_set_air_friction(JiPhysicsWorld* world, float friction);

/** Set time scale (1.0 = real time, 0.5 = slow motion). */
JI_API void ji_physics_world_set_time_scale(JiPhysicsWorld* world, float scale);

/** Step the simulation forward by delta_time seconds. */
JI_API void ji_physics_world_step(JiPhysicsWorld* world, float delta_time);

/* =========================================================================
 * Physics Body
 * ========================================================================= */

/** Create a new physics body. */
JI_API JiPhysicsBody* ji_physics_body_new(float x, float y, float mass);

/** Destroy a physics body. */
JI_API void ji_physics_body_destroy(JiPhysicsBody* body);

/** Set body position. */
JI_API void ji_physics_body_set_position(JiPhysicsBody* body, float x, float y);

/** Set body velocity. */
JI_API void ji_physics_body_set_velocity(JiPhysicsBody* body, float vx, float vy);

/** Apply a force to a body. */
JI_API void ji_physics_body_apply_force(JiPhysicsBody* body, float fx, float fy);

/** Apply an impulse (instantaneous velocity change). */
JI_API void ji_physics_body_apply_impulse(JiPhysicsBody* body, float ix, float iy);

/** Set body as static (immovable). */
JI_API void ji_physics_body_set_static(JiPhysicsBody* body, bool is_static);

/** Set restitution (bounciness). */
JI_API void ji_physics_body_set_restitution(JiPhysicsBody* body, float restitution);

/** Set friction. */
JI_API void ji_physics_body_set_friction(JiPhysicsBody* body, float friction);

/** Set collision radius. */
JI_API void ji_physics_body_set_radius(JiPhysicsBody* body, float radius);

/** Get body position. */
JI_API void ji_physics_body_position(const JiPhysicsBody* body, float* x, float* y);

/** Get body velocity. */
JI_API void ji_physics_body_velocity(const JiPhysicsBody* body, float* vx, float* vy);

/* =========================================================================
 * Spring
 * ========================================================================= */

/** Create a spring between two bodies. */
JI_API JiSpring* ji_spring_new(JiPhysicsBody* a, JiPhysicsBody* b,
                                  float rest_length, float stiffness, float damping);

/** Destroy a spring. */
JI_API void ji_spring_destroy(JiSpring* spring);

/** Add a spring to the world. */
JI_API bool ji_physics_world_add_spring(JiPhysicsWorld* world, JiSpring* spring);

/* =========================================================================
 * Body Registration
 * ========================================================================= */

/** Add a body to the world. */
JI_API bool ji_physics_world_add_body(JiPhysicsWorld* world, JiPhysicsBody* body);

/** Remove a body from the world. */
JI_API bool ji_physics_world_remove_body(JiPhysicsWorld* world, JiPhysicsBody* body);

/** Get body count. */
JI_API int ji_physics_world_body_count(const JiPhysicsWorld* world);

/* =========================================================================
 * Spring Physics (standalone — no world needed)
 * ========================================================================= */

/**
 * Critically damped spring for smooth UI animation.
 * Computes the new position/velocity for a spring system.
 *
 * @param current     Current position.
 * @param velocity    Current velocity (in/out).
 * @param target      Target position.
 * @param stiffness   Spring stiffness (higher = faster).
 * @param damping     Damping ratio (1 = critical, <1 = underdamped, >1 = overdamped).
 * @param dt          Time step.
 * @return            New position.
 */
JI_API float ji_spring_animate(float current, float* velocity, float target,
                                  float stiffness, float damping, float dt);

/**
 * Elastic ease-out with overshoot.
 *
 * @param t       Normalized time (0..1).
 * @param amplitude Overshoot amplitude.
 * @return        Eased value.
 */
JI_API float ji_elastic_ease_out(float t, float amplitude);

/* =========================================================================
 * Gesture Physics — momentum and friction
 * ========================================================================= */

/**
 * Apply momentum with friction to a scrolling/gesture velocity.
 *
 * @param velocity   Current velocity (in/out).
 * @param friction   Friction coefficient (higher = stops faster).
 * @param dt         Time step.
 * @return           True if still moving, false if stopped.
 */
JI_API bool ji_gesture_momentum(float* velocity, float friction, float dt);

/* =========================================================================
 * Collision
 * ========================================================================= */

/** Check if two bodies collide (circle-circle). */
JI_API bool ji_physics_bodies_collide(const JiPhysicsBody* a, const JiPhysicsBody* b);

/** Resolve collision between two bodies (elastic collision response). */
JI_API void ji_physics_resolve_collision(JiPhysicsBody* a, JiPhysicsBody* b);

/* =========================================================================
 * Chain Physics — connected bodies
 * ========================================================================= */

/**
 * Create a chain of connected bodies.
 *
 * @param world       Physics world.
 * @param start_x     Starting X position.
 * @param start_y     Starting Y position.
 * @param link_count  Number of links.
 * @param link_length Distance between links.
 * @param mass        Mass of each link.
 * @param stiffness   Spring stiffness connecting links.
 * @return            Array of body pointers (caller owns, NULL-terminated).
 */
JI_API JiPhysicsBody** ji_physics_chain_new(JiPhysicsWorld* world,
                                               float start_x, float start_y,
                                               int link_count, float link_length,
                                               float mass, float stiffness);

/** Destroy a chain created by ji_physics_chain_new. */
JI_API void ji_physics_chain_destroy(JiPhysicsBody** chain);

/* =========================================================================
 * Fluid Animation — wave propagation on a 1D height field
 * ========================================================================= */

typedef struct JiFluidField {
    float*  heights;     /* Current heights */
    float*  velocities;  /* Vertical velocities */
    float*  prev_heights; /* Previous frame heights */
    int     count;       /* Number of points */
    float   tension;     /* Spring tension (wave speed) */
    float   damping;     /* Wave damping */
} JiFluidField;

/** Create a 1D fluid field. */
JI_API JiFluidField* ji_fluid_new(int count, float tension, float damping);

/** Destroy a fluid field. */
JI_API void ji_fluid_destroy(JiFluidField* fluid);

/** Step the fluid simulation. */
JI_API void ji_fluid_step(JiFluidField* fluid, float dt);

/** Disturb the fluid at a given index with a given force. */
JI_API void ji_fluid_disturb(JiFluidField* fluid, int index, float force);

/** Get height at index. */
JI_API float ji_fluid_height(const JiFluidField* fluid, int index);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_PHYSICS_H */
