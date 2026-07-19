/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_physics.c
 * @brief Physics-based animation engine implementation — semi-implicit Euler
 *        integration, spring forces, collision response, chain physics,
 *        and 1D fluid wave propagation.
 */

#include "jiui/ji_physics.h"
#include "jiui/ji_memory.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* =========================================================================
 * Helpers
 * ========================================================================= */

static float physics_clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* =========================================================================
 * Physics World — Lifecycle
 * ========================================================================= */

JI_API JiPhysicsWorld* ji_physics_world_new(void) {
    JiPhysicsWorld* world = JI_NEW(JiPhysicsWorld);
    if (!world) return NULL;

    world->body_capacity = 64;
    world->bodies = JI_NEW_ARRAY(JiPhysicsBody*, world->body_capacity);
    if (!world->bodies) { ji_free(world); return NULL; }
    world->body_count = 0;

    world->spring_capacity = 32;
    world->springs = JI_NEW_ARRAY(JiSpring, world->spring_capacity);
    if (!world->springs) { ji_free(world->bodies); ji_free(world); return NULL; }
    world->spring_count = 0;

    world->gravity_x = 0.0f;
    world->gravity_y = 980.0f; /* ~1g in pixels/s^2 */
    world->world_bounds_x = 0;
    world->world_bounds_y = 0;
    world->world_bounds_w = 1920;
    world->world_bounds_h = 1080;
    world->air_friction = 0.1f;
    world->time_scale = 1.0f;
    return world;
}

JI_API void ji_physics_world_destroy(JiPhysicsWorld* world) {
    if (!world) return;
    if (world->bodies) ji_free(world->bodies);
    if (world->springs) ji_free(world->springs);
    ji_free(world);
}

JI_API void ji_physics_world_set_gravity(JiPhysicsWorld* world, float gx, float gy) {
    if (!world) return;
    world->gravity_x = gx;
    world->gravity_y = gy;
}

JI_API void ji_physics_world_set_bounds(JiPhysicsWorld* world,
                                           float x, float y, float w, float h) {
    if (!world) return;
    world->world_bounds_x = x;
    world->world_bounds_y = y;
    world->world_bounds_w = w;
    world->world_bounds_h = h;
}

JI_API void ji_physics_world_set_air_friction(JiPhysicsWorld* world, float friction) {
    if (!world) return;
    world->air_friction = physics_clampf(friction, 0.0f, 1.0f);
}

JI_API void ji_physics_world_set_time_scale(JiPhysicsWorld* world, float scale) {
    if (!world) return;
    world->time_scale = physics_clampf(scale, 0.0f, 10.0f);
}

/* =========================================================================
 * Physics World — Simulation Step
 * ========================================================================= */

JI_API void ji_physics_world_step(JiPhysicsWorld* world, float delta_time) {
    if (!world || delta_time <= 0.0f) return;

    float dt = delta_time * world->time_scale;
    float air_damp = 1.0f - world->air_friction * dt;

    /* Apply spring forces */
    for (int i = 0; i < world->spring_count; i++) {
        JiSpring* s = &world->springs[i];
        if (!s->body_a || !s->body_b) continue;

        float dx = s->body_b->x - s->body_a->x;
        float dy = s->body_b->y - s->body_a->y;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist < 0.0001f) continue;

        float stretch = dist - s->rest_length;
        float force = s->stiffness * stretch;

        /* Normalized direction */
        float nx = dx / dist;
        float ny = dy / dist;

        /* Relative velocity for damping */
        float rvx = s->body_b->vx - s->body_a->vx;
        float rvy = s->body_b->vy - s->body_a->vy;
        float damp_force = s->damping * (rvx * nx + rvy * ny);

        float total_force = force + damp_force;
        float fx = nx * total_force;
        float fy = ny * total_force;

        /* Apply equal and opposite forces */
        if (!s->body_a->is_static) {
            s->body_a->ax += fx * s->body_a->inv_mass;
            s->body_a->ay += fy * s->body_a->inv_mass;
        }
        if (!s->body_b->is_static) {
            s->body_b->ax -= fx * s->body_b->inv_mass;
            s->body_b->ay -= fy * s->body_b->inv_mass;
        }
    }

    /* Integrate (semi-implicit Euler) */
    for (int i = 0; i < world->body_count; i++) {
        JiPhysicsBody* b = world->bodies[i];
        if (!b || b->is_static) continue;

        /* Apply gravity */
        b->ax += world->gravity_x * b->inv_mass;
        b->ay += world->gravity_y * b->inv_mass;

        /* Update velocity */
        b->vx += b->ax * dt;
        b->vy += b->ay * dt;

        /* Air friction */
        b->vx *= air_damp;
        b->vy *= air_damp;

        /* Update position */
        b->x += b->vx * dt;
        b->y += b->vy * dt;

        /* Reset acceleration */
        b->ax = 0;
        b->ay = 0;

        /* World bounds collision */
        float r = b->radius;
        if (b->x - r < world->world_bounds_x) {
            b->x = world->world_bounds_x + r;
            b->vx = -b->vx * b->restitution;
        }
        if (b->x + r > world->world_bounds_x + world->world_bounds_w) {
            b->x = world->world_bounds_x + world->world_bounds_w - r;
            b->vx = -b->vx * b->restitution;
        }
        if (b->y - r < world->world_bounds_y) {
            b->y = world->world_bounds_y + r;
            b->vy = -b->vy * b->restitution;
        }
        if (b->y + r > world->world_bounds_y + world->world_bounds_h) {
            b->y = world->world_bounds_y + world->world_bounds_h - r;
            b->vy = -b->vy * b->restitution;
            /* Apply friction on ground */
            b->vx *= (1.0f - b->friction);
        }
    }

    /* Body-body collision */
    for (int i = 0; i < world->body_count; i++) {
        for (int j = i + 1; j < world->body_count; j++) {
            JiPhysicsBody* a = world->bodies[i];
            JiPhysicsBody* b = world->bodies[j];
            if (!a || !b) continue;
            if (a->is_static && b->is_static) continue;
            if (ji_physics_bodies_collide(a, b)) {
                ji_physics_resolve_collision(a, b);
            }
        }
    }
}

/* =========================================================================
 * Physics Body
 * ========================================================================= */

JI_API JiPhysicsBody* ji_physics_body_new(float x, float y, float mass) {
    JiPhysicsBody* body = JI_NEW(JiPhysicsBody);
    if (!body) return NULL;
    memset(body, 0, sizeof(JiPhysicsBody));
    body->x = x;
    body->y = y;
    body->mass = mass;
    body->inv_mass = (mass > 0.0f) ? 1.0f / mass : 0.0f;
    body->restitution = 0.5f;
    body->friction = 0.1f;
    body->radius = 10.0f;
    body->is_static = false;
    return body;
}

JI_API void ji_physics_body_destroy(JiPhysicsBody* body) {
    if (!body) return;
    ji_free(body);
}

JI_API void ji_physics_body_set_position(JiPhysicsBody* body, float x, float y) {
    if (!body) return;
    body->x = x;
    body->y = y;
}

JI_API void ji_physics_body_set_velocity(JiPhysicsBody* body, float vx, float vy) {
    if (!body) return;
    body->vx = vx;
    body->vy = vy;
}

JI_API void ji_physics_body_apply_force(JiPhysicsBody* body, float fx, float fy) {
    if (!body || body->is_static) return;
    body->ax += fx * body->inv_mass;
    body->ay += fy * body->inv_mass;
}

JI_API void ji_physics_body_apply_impulse(JiPhysicsBody* body, float ix, float iy) {
    if (!body || body->is_static) return;
    body->vx += ix * body->inv_mass;
    body->vy += iy * body->inv_mass;
}

JI_API void ji_physics_body_set_static(JiPhysicsBody* body, bool is_static) {
    if (!body) return;
    body->is_static = is_static;
    body->inv_mass = is_static ? 0.0f : (body->mass > 0.0f ? 1.0f / body->mass : 0.0f);
}

JI_API void ji_physics_body_set_restitution(JiPhysicsBody* body, float restitution) {
    if (!body) return;
    body->restitution = physics_clampf(restitution, 0.0f, 1.0f);
}

JI_API void ji_physics_body_set_friction(JiPhysicsBody* body, float friction) {
    if (!body) return;
    body->friction = physics_clampf(friction, 0.0f, 1.0f);
}

JI_API void ji_physics_body_set_radius(JiPhysicsBody* body, float radius) {
    if (!body) return;
    body->radius = radius;
}

JI_API void ji_physics_body_position(const JiPhysicsBody* body, float* x, float* y) {
    if (!body || !x || !y) return;
    *x = body->x;
    *y = body->y;
}

JI_API void ji_physics_body_velocity(const JiPhysicsBody* body, float* vx, float* vy) {
    if (!body || !vx || !vy) return;
    *vx = body->vx;
    *vy = body->vy;
}

/* =========================================================================
 * Spring
 * ========================================================================= */

JI_API JiSpring* ji_spring_new(JiPhysicsBody* a, JiPhysicsBody* b,
                                  float rest_length, float stiffness, float damping) {
    if (!a || !b) return NULL;
    JiSpring* spring = JI_NEW(JiSpring);
    if (!spring) return NULL;
    spring->body_a = a;
    spring->body_b = b;
    spring->rest_length = rest_length;
    spring->stiffness = stiffness;
    spring->damping = damping;
    return spring;
}

JI_API void ji_spring_destroy(JiSpring* spring) {
    if (!spring) return;
    ji_free(spring);
}

JI_API bool ji_physics_world_add_spring(JiPhysicsWorld* world, JiSpring* spring) {
    if (!world || !spring) return false;
    if (world->spring_count >= world->spring_capacity) {
        int new_cap = world->spring_capacity * 2;
        JiSpring* new_arr = (JiSpring*)ji_realloc(world->springs,
                                (size_t)new_cap * sizeof(JiSpring));
        if (!new_arr) return false;
        world->springs = new_arr;
        world->spring_capacity = new_cap;
    }
    world->springs[world->spring_count++] = *spring;
    return true;
}

/* =========================================================================
 * Body Registration
 * ========================================================================= */

JI_API bool ji_physics_world_add_body(JiPhysicsWorld* world, JiPhysicsBody* body) {
    if (!world || !body) return false;
    for (int i = 0; i < world->body_count; i++) {
        if (world->bodies[i] == body) return true;
    }
    if (world->body_count >= world->body_capacity) {
        int new_cap = world->body_capacity * 2;
        JiPhysicsBody** new_arr = (JiPhysicsBody**)ji_realloc(world->bodies,
                                (size_t)new_cap * sizeof(JiPhysicsBody*));
        if (!new_arr) return false;
        world->bodies = new_arr;
        world->body_capacity = new_cap;
    }
    world->bodies[world->body_count++] = body;
    return true;
}

JI_API bool ji_physics_world_remove_body(JiPhysicsWorld* world, JiPhysicsBody* body) {
    if (!world || !body) return false;
    for (int i = 0; i < world->body_count; i++) {
        if (world->bodies[i] == body) {
            for (int j = i; j < world->body_count - 1; j++) {
                world->bodies[j] = world->bodies[j + 1];
            }
            world->body_count--;
            return true;
        }
    }
    return false;
}

JI_API int ji_physics_world_body_count(const JiPhysicsWorld* world) {
    return world ? world->body_count : 0;
}

/* =========================================================================
 * Spring Physics (standalone)
 * ========================================================================= */

JI_API float ji_spring_animate(float current, float* velocity, float target,
                                  float stiffness, float damping, float dt) {
    if (!velocity || dt <= 0.0f) return current;

    /* Semi-implicit Euler for spring-damper system */
    float force = -stiffness * (current - target) - damping * (*velocity);
    *velocity += force * dt;
    return current + (*velocity) * dt;
}

JI_API float ji_elastic_ease_out(float t, float amplitude) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    float p = 0.3f; /* period */
    float s = p / 4.0f;
    return amplitude * powf(2.0f, -10.0f * t) * sinf((t - s) * (2.0f * 3.14159265f) / p) + 1.0f;
}

/* =========================================================================
 * Gesture Physics — momentum and friction
 * ========================================================================= */

JI_API bool ji_gesture_momentum(float* velocity, float friction, float dt) {
    if (!velocity) return false;
    /* Exponential decay: v *= exp(-friction * dt) */
    float decay = expf(-friction * dt);
    *velocity *= decay;
    /* Stop if velocity is very small */
    if (fabsf(*velocity) < 0.5f) {
        *velocity = 0.0f;
        return false;
    }
    return true;
}

/* =========================================================================
 * Collision
 * ========================================================================= */

JI_API bool ji_physics_bodies_collide(const JiPhysicsBody* a, const JiPhysicsBody* b) {
    if (!a || !b) return false;
    float dx = b->x - a->x;
    float dy = b->y - a->y;
    float dist_sq = dx * dx + dy * dy;
    float r_sum = a->radius + b->radius;
    return dist_sq < r_sum * r_sum;
}

JI_API void ji_physics_resolve_collision(JiPhysicsBody* a, JiPhysicsBody* b) {
    if (!a || !b) return;

    float dx = b->x - a->x;
    float dy = b->y - a->y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < 0.0001f) return;

    float nx = dx / dist;
    float ny = dy / dist;

    /* Relative velocity */
    float rvx = b->vx - a->vx;
    float rvy = b->vy - a->vy;
    float vel_along_normal = rvx * nx + rvy * ny;

    /* Don't resolve if separating */
    if (vel_along_normal > 0.0f) return;

    /* Restitution */
    float e = (a->restitution < b->restitution) ? a->restitution : b->restitution;

    /* Impulse scalar */
    float inv_mass_sum = a->inv_mass + b->inv_mass;
    if (inv_mass_sum <= 0.0f) return;

    float j = -(1.0f + e) * vel_along_normal / inv_mass_sum;

    /* Apply impulse */
    float ix = j * nx;
    float iy = j * ny;
    if (!a->is_static) {
        a->vx -= ix * a->inv_mass;
        a->vy -= iy * a->inv_mass;
    }
    if (!b->is_static) {
        b->vx += ix * b->inv_mass;
        b->vy += iy * b->inv_mass;
    }

    /* Positional correction (push apart) */
    float r_sum = a->radius + b->radius;
    float penetration = r_sum - dist;
    if (penetration > 0.0f) {
        float correction = penetration / inv_mass_sum * 0.8f;
        float cx = nx * correction;
        float cy = ny * correction;
        if (!a->is_static) {
            a->x -= cx * a->inv_mass;
            a->y -= cy * a->inv_mass;
        }
        if (!b->is_static) {
            b->x += cx * b->inv_mass;
            b->y += cy * b->inv_mass;
        }
    }
}

/* =========================================================================
 * Chain Physics
 * ========================================================================= */

JI_API JiPhysicsBody** ji_physics_chain_new(JiPhysicsWorld* world,
                                               float start_x, float start_y,
                                               int link_count, float link_length,
                                               float mass, float stiffness) {
    if (!world || link_count <= 0) return NULL;

    JiPhysicsBody** chain = JI_NEW_ARRAY(JiPhysicsBody*, link_count + 1);
    if (!chain) return NULL;

    for (int i = 0; i < link_count; i++) {
        chain[i] = ji_physics_body_new(start_x, start_y + i * link_length, mass);
        if (!chain[i]) {
            /* Cleanup on failure */
            for (int j = 0; j < i; j++) ji_physics_body_destroy(chain[j]);
            ji_free(chain);
            return NULL;
        }
        ji_physics_body_set_radius(chain[i], link_length * 0.4f);
        ji_physics_world_add_body(world, chain[i]);

        /* Connect to previous link with a spring */
        if (i > 0) {
            JiSpring* spring = ji_spring_new(chain[i - 1], chain[i],
                                                link_length, stiffness, 5.0f);
            ji_physics_world_add_spring(world, spring);
            ji_spring_destroy(spring); /* world stores a copy */
        }
    }
    chain[link_count] = NULL; /* NULL-terminate */
    return chain;
}

JI_API void ji_physics_chain_destroy(JiPhysicsBody** chain) {
    if (!chain) return;
    for (int i = 0; chain[i] != NULL; i++) {
        ji_physics_body_destroy(chain[i]);
    }
    ji_free(chain);
}

/* =========================================================================
 * Fluid Animation — 1D wave propagation
 * ========================================================================= */

JI_API JiFluidField* ji_fluid_new(int count, float tension, float damping) {
    if (count <= 0) return NULL;
    JiFluidField* fluid = JI_NEW(JiFluidField);
    if (!fluid) return NULL;

    fluid->count = count;
    fluid->heights = JI_NEW_ARRAY(float, count);
    fluid->velocities = JI_NEW_ARRAY(float, count);
    fluid->prev_heights = JI_NEW_ARRAY(float, count);
    if (!fluid->heights || !fluid->velocities || !fluid->prev_heights) {
        ji_free(fluid->heights);
        ji_free(fluid->velocities);
        ji_free(fluid->prev_heights);
        ji_free(fluid);
        return NULL;
    }
    memset(fluid->heights, 0, sizeof(float) * count);
    memset(fluid->velocities, 0, sizeof(float) * count);
    memset(fluid->prev_heights, 0, sizeof(float) * count);
    fluid->tension = tension;
    fluid->damping = damping;
    return fluid;
}

JI_API void ji_fluid_destroy(JiFluidField* fluid) {
    if (!fluid) return;
    ji_free(fluid->heights);
    ji_free(fluid->velocities);
    ji_free(fluid->prev_heights);
    ji_free(fluid);
}

JI_API void ji_fluid_step(JiFluidField* fluid, float dt) {
    if (!fluid || dt <= 0.0f) return;

    int n = fluid->count;

    /* Save current heights as previous */
    memcpy(fluid->prev_heights, fluid->heights, sizeof(float) * n);

    /* Wave equation: h_new[i] = 2*h[i] - h_prev[i] + tension * (h[i-1] + h[i+1] - 2*h[i]) */
    for (int i = 0; i < n; i++) {
        float left  = (i > 0) ? fluid->heights[i - 1] : fluid->heights[i];
        float right = (i < n - 1) ? fluid->heights[i + 1] : fluid->heights[i];
        float laplacian = left + right - 2.0f * fluid->heights[i];
        float new_h = 2.0f * fluid->heights[i] - fluid->prev_heights[i]
                      + fluid->tension * laplacian;
        /* Apply damping */
        new_h *= (1.0f - fluid->damping * dt);
        fluid->velocities[i] = new_h - fluid->heights[i];
        fluid->prev_heights[i] = fluid->heights[i]; /* store old for next step */
        fluid->heights[i] = new_h;
    }
}

JI_API void ji_fluid_disturb(JiFluidField* fluid, int index, float force) {
    if (!fluid || index < 0 || index >= fluid->count) return;
    fluid->heights[index] += force;
}

JI_API float ji_fluid_height(const JiFluidField* fluid, int index) {
    if (!fluid || index < 0 || index >= fluid->count) return 0.0f;
    return fluid->heights[index];
}
