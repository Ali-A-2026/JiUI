/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_sandbox.h
 * @brief Secure UI sandbox — widget isolation, permission system,
 *        safe plugins, memory boundaries (arena per widget group).
 */

#ifndef JIUI_SANDBOX_H
#define JIUI_SANDBOX_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Sandbox Constants
 * ========================================================================= */

#define JI_SANDBOX_MAX_GROUPS     64
#define JI_SANDBOX_MAX_PERMS     128
#define JI_SANDBOX_ARENA_SIZE  (4 * 1024 * 1024)   /* 4 MB default arena */

/* =========================================================================
 * Permission Flags
 * ========================================================================= */

typedef enum JiSandboxPermission {
    JI_PERM_NONE        = 0,
    JI_PERM_FILE_READ   = 1 << 0,
    JI_PERM_FILE_WRITE  = 1 << 1,
    JI_PERM_NETWORK     = 1 << 2,
    JI_PERM_CLIPBOARD   = 1 << 3,
    JI_PERM_PROCESS     = 1 << 4,
    JI_PERM_AUDIO       = 1 << 5,
    JI_PERM_CAMERA      = 1 << 6,
    JI_PERM_LOCATION    = 1 << 7,
    JI_PERM_ALL         = 0xFFFFFFFF
} JiSandboxPermission;

/* =========================================================================
 * Sandbox Arena (memory boundary)
 * ========================================================================= */

typedef struct JiSandboxArena {
    void*    base;          /* Base pointer */
    size_t   size;          /* Total size */
    size_t   offset;        /* Current allocation offset */
    size_t   peak;          /* Peak usage */
    int      group_id;      /* Owning group */
} JiSandboxArena;

/* =========================================================================
 * Sandbox Group
 * ========================================================================= */

typedef struct JiSandboxGroup {
    int      id;            /* Group ID */
    char     name[64];      /* Group name */
    uint32_t permissions;   /* Bitmask of JiSandboxPermission */
    JiSandboxArena arena;   /* Memory arena for this group */
    bool     active;
} JiSandboxGroup;

/* =========================================================================
 * Sandbox Manager
 * ========================================================================= */

typedef struct JiSandboxManager {
    JiSandboxGroup groups[JI_SANDBOX_MAX_GROUPS];
    int            group_count;
    bool           enforce;   /* If true, violations cause errors */
} JiSandboxManager;

/* =========================================================================
 * Sandbox API — Lifecycle
 * ========================================================================= */

JI_API JiSandboxManager* ji_sandbox_new(void);
JI_API void              ji_sandbox_free(JiSandboxManager* mgr);

/* =========================================================================
 * Sandbox API — Group Management
 * ========================================================================= */

JI_API int   ji_sandbox_create_group(JiSandboxManager* mgr, const char* name,
                                       size_t arena_size);
JI_API int   ji_sandbox_destroy_group(JiSandboxManager* mgr, int group_id);
JI_API JiSandboxGroup* ji_sandbox_get_group(JiSandboxManager* mgr, int group_id);
JI_API int   ji_sandbox_get_group_count(const JiSandboxManager* mgr);

/* =========================================================================
 * Sandbox API — Permissions
 * ========================================================================= */

JI_API int   ji_sandbox_grant_permission(JiSandboxManager* mgr, int group_id,
                                            JiSandboxPermission perm);
JI_API int   ji_sandbox_revoke_permission(JiSandboxManager* mgr, int group_id,
                                             JiSandboxPermission perm);
JI_API bool  ji_sandbox_has_permission(const JiSandboxManager* mgr, int group_id,
                                          JiSandboxPermission perm);
JI_API uint32_t ji_sandbox_get_permissions(const JiSandboxManager* mgr, int group_id);

/* =========================================================================
 * Sandbox API — Arena Allocation
 * ========================================================================= */

JI_API void* ji_sandbox_alloc(JiSandboxManager* mgr, int group_id, size_t size);
JI_API void  ji_sandbox_arena_reset(JiSandboxManager* mgr, int group_id);
JI_API size_t ji_sandbox_arena_get_used(const JiSandboxManager* mgr, int group_id);
JI_API size_t ji_sandbox_arena_get_peak(const JiSandboxManager* mgr, int group_id);
JI_API size_t ji_sandbox_arena_get_total(const JiSandboxManager* mgr, int group_id);

/* =========================================================================
 * Sandbox API — Enforcement
 * ========================================================================= */

JI_API void  ji_sandbox_set_enforce(JiSandboxManager* mgr, bool enforce);
JI_API bool  ji_sandbox_get_enforce(const JiSandboxManager* mgr);

/* =========================================================================
 * Sandbox API — Permission Check (for operations)
 * ========================================================================= */

JI_API bool  ji_sandbox_check_file_read(const JiSandboxManager* mgr, int group_id);
JI_API bool  ji_sandbox_check_file_write(const JiSandboxManager* mgr, int group_id);
JI_API bool  ji_sandbox_check_network(const JiSandboxManager* mgr, int group_id);
JI_API bool  ji_sandbox_check_clipboard(const JiSandboxManager* mgr, int group_id);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_SANDBOX_H */
