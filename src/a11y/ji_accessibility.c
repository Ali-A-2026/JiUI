/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_accessibility.c
 * @brief Accessibility system implementation.
 */

#include "jiui/ji_accessibility.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* =========================================================================
 * Lifecycle
 * ========================================================================= */

JiA11yManager* ji_a11y_new(void)
{
    JiA11yManager* mgr = (JiA11yManager*)calloc(1, sizeof(JiA11yManager));
    if (!mgr) return NULL;
    mgr->next_id = 1;       /* 0 = invalid */
    mgr->focused_node = 0;
    mgr->magnification = 1;
    return mgr;
}

void ji_a11y_free(JiA11yManager* mgr)
{
    if (mgr) free(mgr);
}

/* =========================================================================
 * Node Management
 * ========================================================================= */

static JiA11yNode* find_node(JiA11yManager* mgr, uint32_t id)
{
    for (int i = 0; i < mgr->node_count; i++) {
        if (mgr->nodes[i].id == id) return &mgr->nodes[i];
    }
    return NULL;
}

uint32_t ji_a11y_register_node(JiA11yManager* mgr, uint32_t parent_id,
                                  JiA11yRole role, const char* name)
{
    if (!mgr) return 0;
    if (mgr->node_count >= JI_A11Y_MAX_WIDGETS) return 0;
    JiA11yNode* node = &mgr->nodes[mgr->node_count++];
    memset(node, 0, sizeof(*node));
    node->id = mgr->next_id++;
    node->parent_id = parent_id;
    node->role = role;
    node->visible = true;
    node->tab_index = -1;
    if (name) {
        strncpy(node->name, name, JI_A11Y_MAX_NAME_LEN - 1);
        node->name[JI_A11Y_MAX_NAME_LEN - 1] = '\0';
    }
    return node->id;
}

int ji_a11y_unregister_node(JiA11yManager* mgr, uint32_t id)
{
    if (!mgr) return -1;
    for (int i = 0; i < mgr->node_count; i++) {
        if (mgr->nodes[i].id == id) {
            /* Unfocus if focused */
            if (mgr->focused_node == id) mgr->focused_node = 0;
            /* Re-parent children to this node's parent */
            for (int j = 0; j < mgr->node_count; j++) {
                if (mgr->nodes[j].parent_id == id)
                    mgr->nodes[j].parent_id = mgr->nodes[i].parent_id;
            }
            /* Remove by swapping with last */
            mgr->nodes[i] = mgr->nodes[mgr->node_count - 1];
            mgr->node_count--;
            return 0;
        }
    }
    return -1;
}

JiA11yNode* ji_a11y_get_node(JiA11yManager* mgr, uint32_t id)
{
    if (!mgr) return NULL;
    return find_node(mgr, id);
}

int ji_a11y_set_name(JiA11yManager* mgr, uint32_t id, const char* name)
{
    JiA11yNode* node = find_node(mgr, id);
    if (!node || !name) return -1;
    strncpy(node->name, name, JI_A11Y_MAX_NAME_LEN - 1);
    node->name[JI_A11Y_MAX_NAME_LEN - 1] = '\0';
    return 0;
}

int ji_a11y_set_description(JiA11yManager* mgr, uint32_t id, const char* desc)
{
    JiA11yNode* node = find_node(mgr, id);
    if (!node) return -1;
    if (desc) {
        strncpy(node->description, desc, JI_A11Y_MAX_DESC_LEN - 1);
        node->description[JI_A11Y_MAX_DESC_LEN - 1] = '\0';
    } else {
        node->description[0] = '\0';
    }
    return 0;
}

int ji_a11y_set_value(JiA11yManager* mgr, uint32_t id, const char* value)
{
    JiA11yNode* node = find_node(mgr, id);
    if (!node) return -1;
    if (value) {
        strncpy(node->value, value, sizeof(node->value) - 1);
        node->value[sizeof(node->value) - 1] = '\0';
    } else {
        node->value[0] = '\0';
    }
    return 0;
}

int ji_a11y_set_role(JiA11yManager* mgr, uint32_t id, JiA11yRole role)
{
    JiA11yNode* node = find_node(mgr, id);
    if (!node) return -1;
    node->role = role;
    return 0;
}

int ji_a11y_set_state(JiA11yManager* mgr, uint32_t id, uint32_t states)
{
    JiA11yNode* node = find_node(mgr, id);
    if (!node) return -1;
    node->states = states;
    return 0;
}

int ji_a11y_add_state(JiA11yManager* mgr, uint32_t id, JiA11yState state)
{
    JiA11yNode* node = find_node(mgr, id);
    if (!node) return -1;
    node->states |= (uint32_t)state;
    return 0;
}

int ji_a11y_remove_state(JiA11yManager* mgr, uint32_t id, JiA11yState state)
{
    JiA11yNode* node = find_node(mgr, id);
    if (!node) return -1;
    node->states &= ~(uint32_t)state;
    return 0;
}

int ji_a11y_set_bounds(JiA11yManager* mgr, uint32_t id, JiRect bounds)
{
    JiA11yNode* node = find_node(mgr, id);
    if (!node) return -1;
    node->bounds = bounds;
    return 0;
}

int ji_a11y_set_tab_index(JiA11yManager* mgr, uint32_t id, int tab_index)
{
    JiA11yNode* node = find_node(mgr, id);
    if (!node) return -1;
    node->tab_index = tab_index;
    return 0;
}

/* =========================================================================
 * Focus & Navigation
 * ========================================================================= */

int ji_a11y_focus(JiA11yManager* mgr, uint32_t id)
{
    if (!mgr) return -1;
    /* Remove focused state from old node */
    if (mgr->focused_node != 0) {
        JiA11yNode* old = find_node(mgr, mgr->focused_node);
        if (old) old->states &= ~(uint32_t)JI_A11Y_STATE_FOCUSED;
    }
    /* Focus new node */
    JiA11yNode* node = find_node(mgr, id);
    if (!node) return -1;
    node->states |= (uint32_t)JI_A11Y_STATE_FOCUSED;
    mgr->focused_node = id;
    return 0;
}

uint32_t ji_a11y_get_focused(const JiA11yManager* mgr)
{
    return mgr ? mgr->focused_node : 0;
}

static bool is_focusable(const JiA11yNode* node)
{
    if (!node || !node->visible) return false;
    if (node->states & JI_A11Y_STATE_DISABLED) return false;
    if (node->states & JI_A11Y_STATE_HIDDEN) return false;
    if (node->tab_index < 0) return false;
    return true;
}

uint32_t ji_a11y_focus_next(JiA11yManager* mgr)
{
    if (!mgr) return 0;
    /* Find current tab index */
    int current_tab = -1;
    if (mgr->focused_node != 0) {
        JiA11yNode* cur = find_node(mgr, mgr->focused_node);
        if (cur) current_tab = cur->tab_index;
    }
    /* Find next focusable node with higher tab index */
    uint32_t best_id = 0;
    int best_tab = INT32_MAX;
    for (int i = 0; i < mgr->node_count; i++) {
        const JiA11yNode* n = &mgr->nodes[i];
        if (!is_focusable(n)) continue;
        if (n->tab_index > current_tab && n->tab_index < best_tab) {
            best_tab = n->tab_index;
            best_id = n->id;
        }
    }
    if (best_id == 0) {
        /* Wrap around to first */
        return ji_a11y_focus_first(mgr);
    }
    ji_a11y_focus(mgr, best_id);
    return best_id;
}

uint32_t ji_a11y_focus_prev(JiA11yManager* mgr)
{
    if (!mgr) return 0;
    int current_tab = INT32_MAX;
    if (mgr->focused_node != 0) {
        JiA11yNode* cur = find_node(mgr, mgr->focused_node);
        if (cur) current_tab = cur->tab_index;
    }
    uint32_t best_id = 0;
    int best_tab = -1;
    for (int i = 0; i < mgr->node_count; i++) {
        const JiA11yNode* n = &mgr->nodes[i];
        if (!is_focusable(n)) continue;
        if (n->tab_index < current_tab && n->tab_index > best_tab) {
            best_tab = n->tab_index;
            best_id = n->id;
        }
    }
    if (best_id == 0) {
        return ji_a11y_focus_last(mgr);
    }
    ji_a11y_focus(mgr, best_id);
    return best_id;
}

uint32_t ji_a11y_focus_first(JiA11yManager* mgr)
{
    if (!mgr) return 0;
    uint32_t best_id = 0;
    int best_tab = INT32_MAX;
    for (int i = 0; i < mgr->node_count; i++) {
        const JiA11yNode* n = &mgr->nodes[i];
        if (!is_focusable(n)) continue;
        if (n->tab_index < best_tab) {
            best_tab = n->tab_index;
            best_id = n->id;
        }
    }
    if (best_id) ji_a11y_focus(mgr, best_id);
    return best_id;
}

uint32_t ji_a11y_focus_last(JiA11yManager* mgr)
{
    if (!mgr) return 0;
    uint32_t best_id = 0;
    int best_tab = -1;
    for (int i = 0; i < mgr->node_count; i++) {
        const JiA11yNode* n = &mgr->nodes[i];
        if (!is_focusable(n)) continue;
        if (n->tab_index > best_tab) {
            best_tab = n->tab_index;
            best_id = n->id;
        }
    }
    if (best_id) ji_a11y_focus(mgr, best_id);
    return best_id;
}

/* =========================================================================
 * Announcement
 * ========================================================================= */

int ji_a11y_announce(JiA11yManager* mgr, const char* message)
{
    if (!mgr || !message) return -1;
    if (mgr->announce_count >= JI_A11Y_MAX_ANNOUNCE) return -1;
    /* Store in a simple circular buffer — overwrite oldest */
    /* For simplicity, just store the latest message */
    strncpy(mgr->announcements, message, JI_A11Y_MAX_ANNOUNCE - 1);
    mgr->announcements[JI_A11Y_MAX_ANNOUNCE - 1] = '\0';
    mgr->announce_count = 1;
    return 0;
}

const char* ji_a11y_get_announcement(JiA11yManager* mgr, int index)
{
    if (!mgr || index != 0 || mgr->announce_count == 0) return NULL;
    return mgr->announcements;
}

int ji_a11y_get_announcement_count(const JiA11yManager* mgr)
{
    return mgr ? mgr->announce_count : 0;
}

void ji_a11y_clear_announcements(JiA11yManager* mgr)
{
    if (!mgr) return;
    mgr->announce_count = 0;
    mgr->announcements[0] = '\0';
}

/* =========================================================================
 * Settings
 * ========================================================================= */

void ji_a11y_set_high_contrast(JiA11yManager* mgr, bool enabled)
{
    if (mgr) mgr->high_contrast = enabled;
}

bool ji_a11y_get_high_contrast(const JiA11yManager* mgr)
{
    return mgr ? mgr->high_contrast : false;
}

void ji_a11y_set_screen_reader(JiA11yManager* mgr, bool active)
{
    if (mgr) mgr->screen_reader_active = active;
}

bool ji_a11y_get_screen_reader(const JiA11yManager* mgr)
{
    return mgr ? mgr->screen_reader_active : false;
}

void ji_a11y_set_magnification(JiA11yManager* mgr, int level)
{
    if (mgr && level >= 1) mgr->magnification = level;
}

int ji_a11y_get_magnification(const JiA11yManager* mgr)
{
    return mgr ? mgr->magnification : 1;
}
