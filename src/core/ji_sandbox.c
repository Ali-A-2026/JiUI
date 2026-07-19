/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_sandbox.c
 * @brief Secure UI sandbox implementation.
 */

#include "jiui/ji_sandbox.h"
#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * Lifecycle
 * ========================================================================= */

JiSandboxManager* ji_sandbox_new(void)
{
    JiSandboxManager* mgr = (JiSandboxManager*)calloc(1, sizeof(JiSandboxManager));
    if (!mgr) return NULL;
    mgr->enforce = true;
    return mgr;
}

void ji_sandbox_free(JiSandboxManager* mgr)
{
    if (!mgr) return;
    /* Free all arena memory */
    for (int i = 0; i < mgr->group_count; i++) {
        if (mgr->groups[i].arena.base) {
            free(mgr->groups[i].arena.base);
        }
    }
    free(mgr);
}

/* =========================================================================
 * Group Management
 * ========================================================================= */

int ji_sandbox_create_group(JiSandboxManager* mgr, const char* name, size_t arena_size)
{
    if (!mgr || mgr->group_count >= JI_SANDBOX_MAX_GROUPS) return -1;
    if (arena_size == 0) arena_size = JI_SANDBOX_ARENA_SIZE;
    JiSandboxGroup* group = &mgr->groups[mgr->group_count];
    memset(group, 0, sizeof(*group));
    group->id = mgr->group_count + 1;   /* IDs start at 1 */
    if (name) {
        strncpy(group->name, name, sizeof(group->name) - 1);
        group->name[sizeof(group->name) - 1] = '\0';
    }
    group->permissions = JI_PERM_NONE;
    group->arena.base = malloc(arena_size);
    if (!group->arena.base) return -1;
    group->arena.size = arena_size;
    group->arena.offset = 0;
    group->arena.peak = 0;
    group->arena.group_id = group->id;
    group->active = true;
    return group->id;
}

int ji_sandbox_destroy_group(JiSandboxManager* mgr, int group_id)
{
    if (!mgr) return -1;
    for (int i = 0; i < mgr->group_count; i++) {
        if (mgr->groups[i].id == group_id) {
            if (mgr->groups[i].arena.base) {
                free(mgr->groups[i].arena.base);
            }
            /* Swap with last and decrement */
            mgr->groups[i] = mgr->groups[mgr->group_count - 1];
            mgr->group_count--;
            return 0;
        }
    }
    return -1;
}

JiSandboxGroup* ji_sandbox_get_group(JiSandboxManager* mgr, int group_id)
{
    if (!mgr) return NULL;
    for (int i = 0; i < mgr->group_count; i++) {
        if (mgr->groups[i].id == group_id) return &mgr->groups[i];
    }
    return NULL;
}

int ji_sandbox_get_group_count(const JiSandboxManager* mgr)
{
    return mgr ? mgr->group_count : 0;
}

/* =========================================================================
 * Permissions
 * ========================================================================= */

int ji_sandbox_grant_permission(JiSandboxManager* mgr, int group_id,
                                    JiSandboxPermission perm)
{
    JiSandboxGroup* group = ji_sandbox_get_group(mgr, group_id);
    if (!group) return -1;
    group->permissions |= (uint32_t)perm;
    return 0;
}

int ji_sandbox_revoke_permission(JiSandboxManager* mgr, int group_id,
                                     JiSandboxPermission perm)
{
    JiSandboxGroup* group = ji_sandbox_get_group(mgr, group_id);
    if (!group) return -1;
    group->permissions &= ~(uint32_t)perm;
    return 0;
}

bool ji_sandbox_has_permission(const JiSandboxManager* mgr, int group_id,
                                  JiSandboxPermission perm)
{
    if (!mgr) return false;
    for (int i = 0; i < mgr->group_count; i++) {
        if (mgr->groups[i].id == group_id) {
            return (mgr->groups[i].permissions & (uint32_t)perm) != 0;
        }
    }
    return false;
}

uint32_t ji_sandbox_get_permissions(const JiSandboxManager* mgr, int group_id)
{
    if (!mgr) return 0;
    for (int i = 0; i < mgr->group_count; i++) {
        if (mgr->groups[i].id == group_id)
            return mgr->groups[i].permissions;
    }
    return 0;
}

/* =========================================================================
 * Arena Allocation
 * ========================================================================= */

void* ji_sandbox_alloc(JiSandboxManager* mgr, int group_id, size_t size)
{
    JiSandboxGroup* group = ji_sandbox_get_group(mgr, group_id);
    if (!group || !group->active) return NULL;
    JiSandboxArena* arena = &group->arena;
    /* Align to 8 bytes */
    size_t aligned_offset = (arena->offset + 7) & ~(size_t)7;
    if (aligned_offset + size > arena->size) return NULL;
    void* ptr = (char*)arena->base + aligned_offset;
    arena->offset = aligned_offset + size;
    if (arena->offset > arena->peak) arena->peak = arena->offset;
    return ptr;
}

void ji_sandbox_arena_reset(JiSandboxManager* mgr, int group_id)
{
    JiSandboxGroup* group = ji_sandbox_get_group(mgr, group_id);
    if (!group) return;
    group->arena.offset = 0;
}

size_t ji_sandbox_arena_get_used(const JiSandboxManager* mgr, int group_id)
{
    JiSandboxGroup* group = ji_sandbox_get_group((JiSandboxManager*)mgr, group_id);
    return group ? group->arena.offset : 0;
}

size_t ji_sandbox_arena_get_peak(const JiSandboxManager* mgr, int group_id)
{
    JiSandboxGroup* group = ji_sandbox_get_group((JiSandboxManager*)mgr, group_id);
    return group ? group->arena.peak : 0;
}

size_t ji_sandbox_arena_get_total(const JiSandboxManager* mgr, int group_id)
{
    JiSandboxGroup* group = ji_sandbox_get_group((JiSandboxManager*)mgr, group_id);
    return group ? group->arena.size : 0;
}

/* =========================================================================
 * Enforcement
 * ========================================================================= */

void ji_sandbox_set_enforce(JiSandboxManager* mgr, bool enforce)
{
    if (mgr) mgr->enforce = enforce;
}

bool ji_sandbox_get_enforce(const JiSandboxManager* mgr)
{
    return mgr ? mgr->enforce : false;
}

/* =========================================================================
 * Permission Checks
 * ========================================================================= */

bool ji_sandbox_check_file_read(const JiSandboxManager* mgr, int group_id)
{
    return ji_sandbox_has_permission(mgr, group_id, JI_PERM_FILE_READ);
}

bool ji_sandbox_check_file_write(const JiSandboxManager* mgr, int group_id)
{
    return ji_sandbox_has_permission(mgr, group_id, JI_PERM_FILE_WRITE);
}

bool ji_sandbox_check_network(const JiSandboxManager* mgr, int group_id)
{
    return ji_sandbox_has_permission(mgr, group_id, JI_PERM_NETWORK);
}

bool ji_sandbox_check_clipboard(const JiSandboxManager* mgr, int group_id)
{
    return ji_sandbox_has_permission(mgr, group_id, JI_PERM_CLIPBOARD);
}
