/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_splitter.c
 * @brief Splitter implementation — resizable multi-panel container.
 */

#include <jiui/ji_splitter.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

/* ---- Lifecycle ---- */

JiSplitter* ji_splitter_new(JiSplitterOrientation orientation) {
    JiSplitter* sp = (JiSplitter*)ji_calloc(1, sizeof(JiSplitter));
    if (!sp) {
        JI_ERROR_LOG("ji_splitter_new: out of memory");
        return NULL;
    }

    sp->orientation = orientation;
    sp->panel_count = 0;
    sp->panel_capacity = 4;
    sp->panels = (JiSplitterPanel*)ji_alloc(sizeof(JiSplitterPanel) * sp->panel_capacity);
    sp->handle_width = 4;
    sp->drag_index = -1;
    sp->drag_start_pos = 0;
    sp->drag_start_size = 0;

    return sp;
}

void ji_splitter_destroy(JiSplitter* sp) {
    if (!sp) return;
    ji_free(sp->panels);
    ji_free(sp);
}

/* ---- Internal: grow panels array ---- */

static void grow_panels(JiSplitter* sp) {
    if (sp->panel_count < sp->panel_capacity) return;
    int new_cap = sp->panel_capacity * 2;
    JiSplitterPanel* new_arr = (JiSplitterPanel*)ji_alloc(sizeof(JiSplitterPanel) * new_cap);
    if (!new_arr) return;
    memcpy(new_arr, sp->panels, sizeof(JiSplitterPanel) * sp->panel_count);
    ji_free(sp->panels);
    sp->panels = new_arr;
    sp->panel_capacity = new_cap;
}

/* ---- Panel management ---- */

void ji_splitter_add_panel(JiSplitter* sp, void* content, int min_size, int stretch_factor) {
    if (!sp) return;
    grow_panels(sp);

    JiSplitterPanel* p = &sp->panels[sp->panel_count++];
    p->content = content;
    p->min_size = min_size > 0 ? min_size : 32;
    p->max_size = 0;  /* unlimited */
    p->stretch_factor = stretch_factor > 0 ? stretch_factor : 1;
    p->current_size = 0;
    p->collapsible = false;
    p->is_collapsed = false;
}

void ji_splitter_insert_panel(JiSplitter* sp, int index, void* content,
                                 int min_size, int stretch_factor) {
    if (!sp || index < 0 || index > sp->panel_count) return;
    grow_panels(sp);

    /* Shift panels right */
    for (int i = sp->panel_count; i > index; i--) {
        sp->panels[i] = sp->panels[i - 1];
    }

    JiSplitterPanel* p = &sp->panels[index];
    p->content = content;
    p->min_size = min_size > 0 ? min_size : 32;
    p->max_size = 0;
    p->stretch_factor = stretch_factor > 0 ? stretch_factor : 1;
    p->current_size = 0;
    p->collapsible = false;
    p->is_collapsed = false;
    sp->panel_count++;
}

void ji_splitter_remove_panel(JiSplitter* sp, int index) {
    if (!sp || index < 0 || index >= sp->panel_count) return;

    for (int i = index; i < sp->panel_count - 1; i++) {
        sp->panels[i] = sp->panels[i + 1];
    }
    sp->panel_count--;
}

int ji_splitter_panel_count(const JiSplitter* sp) {
    return sp ? sp->panel_count : 0;
}

JiSplitterPanel* ji_splitter_get_panel(const JiSplitter* sp, int index) {
    if (!sp || index < 0 || index >= sp->panel_count) return NULL;
    return (JiSplitterPanel*)&sp->panels[index];
}

/* ---- Size management ---- */

void ji_splitter_set_sizes(JiSplitter* sp, const int* sizes, int count) {
    if (!sp || !sizes || count != sp->panel_count) return;
    for (int i = 0; i < count; i++) {
        sp->panels[i].current_size = sizes[i];
    }
}

void ji_splitter_get_sizes(const JiSplitter* sp, int* sizes, int count) {
    if (!sp || !sizes) return;
    int n = count < sp->panel_count ? count : sp->panel_count;
    for (int i = 0; i < n; i++) {
        sizes[i] = sp->panels[i].current_size;
    }
}

void ji_splitter_set_stretch_factor(JiSplitter* sp, int index, int factor) {
    if (!sp || index < 0 || index >= sp->panel_count) return;
    sp->panels[index].stretch_factor = factor > 0 ? factor : 1;
}

int ji_splitter_get_stretch_factor(const JiSplitter* sp, int index) {
    if (!sp || index < 0 || index >= sp->panel_count) return 0;
    return sp->panels[index].stretch_factor;
}

void ji_splitter_set_size_constraints(JiSplitter* sp, int index, int min_size, int max_size) {
    if (!sp || index < 0 || index >= sp->panel_count) return;
    sp->panels[index].min_size = min_size > 0 ? min_size : 0;
    sp->panels[index].max_size = max_size > 0 ? max_size : 0;
}

/* ---- Orientation ---- */

void ji_splitter_set_orientation(JiSplitter* sp, JiSplitterOrientation orientation) {
    if (!sp) return;
    sp->orientation = orientation;
}

JiSplitterOrientation ji_splitter_get_orientation(const JiSplitter* sp) {
    return sp ? sp->orientation : JI_SPLITTER_HORIZONTAL;
}

/* ---- Geometry ---- */

static void recalculate_sizes(JiSplitter* sp) {
    if (!sp || sp->panel_count == 0) return;

    int total = (sp->orientation == JI_SPLITTER_HORIZONTAL) ? sp->rect.width : sp->rect.height;
    int handles_total = (sp->panel_count - 1) * sp->handle_width;
    int available = total - handles_total;
    if (available < 0) available = 0;

    /* Calculate total stretch */
    int total_stretch = 0;
    for (int i = 0; i < sp->panel_count; i++) {
        if (!sp->panels[i].is_collapsed) {
            total_stretch += sp->panels[i].stretch_factor;
        }
    }

    /* Distribute space proportionally */
    int remaining = available;
    for (int i = 0; i < sp->panel_count; i++) {
        if (sp->panels[i].is_collapsed) {
            sp->panels[i].current_size = 0;
        } else if (total_stretch > 0) {
            int size = (available * sp->panels[i].stretch_factor) / total_stretch;
            /* Clamp to min */
            if (size < sp->panels[i].min_size) size = sp->panels[i].min_size;
            /* Clamp to max */
            if (sp->panels[i].max_size > 0 && size > sp->panels[i].max_size)
                size = sp->panels[i].max_size;
            sp->panels[i].current_size = size;
            remaining -= size;
        }
    }

    /* Give any remaining space to the last non-collapsed panel */
    if (remaining > 0) {
        for (int i = sp->panel_count - 1; i >= 0; i--) {
            if (!sp->panels[i].is_collapsed) {
                sp->panels[i].current_size += remaining;
                break;
            }
        }
    }
}

void ji_splitter_set_rect(JiSplitter* sp, JiRect rect) {
    if (!sp) return;
    sp->rect = rect;
    recalculate_sizes(sp);
}

JiRect ji_splitter_get_rect(const JiSplitter* sp) {
    return sp ? sp->rect : (JiRect){0, 0, 0, 0};
}

JiRect ji_splitter_get_panel_rect(const JiSplitter* sp, int index) {
    JiRect r = {0, 0, 0, 0};
    if (!sp || index < 0 || index >= sp->panel_count) return r;

    int offset = 0;
    for (int i = 0; i < index; i++) {
        offset += sp->panels[i].current_size + sp->handle_width;
    }

    if (sp->orientation == JI_SPLITTER_HORIZONTAL) {
        r.x = sp->rect.x + offset;
        r.y = sp->rect.y;
        r.width = sp->panels[index].current_size;
        r.height = sp->rect.height;
    } else {
        r.x = sp->rect.x;
        r.y = sp->rect.y + offset;
        r.width = sp->rect.width;
        r.height = sp->panels[index].current_size;
    }
    return r;
}

JiRect ji_splitter_get_handle_rect(const JiSplitter* sp, int handle_index) {
    JiRect r = {0, 0, 0, 0};
    if (!sp || handle_index < 0 || handle_index >= sp->panel_count - 1) return r;

    int offset = 0;
    for (int i = 0; i <= handle_index; i++) {
        offset += sp->panels[i].current_size;
        if (i < handle_index) offset += sp->handle_width;
    }

    if (sp->orientation == JI_SPLITTER_HORIZONTAL) {
        r.x = sp->rect.x + offset;
        r.y = sp->rect.y;
        r.width = sp->handle_width;
        r.height = sp->rect.height;
    } else {
        r.x = sp->rect.x;
        r.y = sp->rect.y + offset;
        r.width = sp->rect.width;
        r.height = sp->handle_width;
    }
    return r;
}

/* ---- Interaction ---- */

bool ji_splitter_begin_drag(JiSplitter* sp, int mx, int my) {
    if (!sp) return false;
    int handle = ji_splitter_hit_test_handle(sp, mx, my);
    if (handle < 0) return false;

    sp->drag_index = handle;
    sp->drag_start_pos = (sp->orientation == JI_SPLITTER_HORIZONTAL) ? mx : my;
    sp->drag_start_size = sp->panels[handle].current_size;
    return true;
}

bool ji_splitter_update_drag(JiSplitter* sp, int mx, int my) {
    if (!sp || sp->drag_index < 0) return false;

    int current_pos = (sp->orientation == JI_SPLITTER_HORIZONTAL) ? mx : my;
    int delta = current_pos - sp->drag_start_pos;

    int idx = sp->drag_index;
    int next_idx = idx + 1;

    /* Resize: move size from panel[idx] to panel[next_idx] or vice versa */
    int new_size = sp->drag_start_size + delta;

    /* Clamp to min sizes */
    int min_this = sp->panels[idx].min_size;
    int min_next = sp->panels[next_idx].min_size;
    int total = sp->panels[idx].current_size + sp->panels[next_idx].current_size;

    if (new_size < min_this) new_size = min_this;
    if (new_size > total - min_next) new_size = total - min_next;

    if (new_size == sp->panels[idx].current_size) return false;

    sp->panels[next_idx].current_size = total - new_size;
    sp->panels[idx].current_size = new_size;
    return true;
}

void ji_splitter_end_drag(JiSplitter* sp) {
    if (!sp) return;
    sp->drag_index = -1;
}

bool ji_splitter_is_dragging(const JiSplitter* sp) {
    return sp ? (sp->drag_index >= 0) : false;
}

int ji_splitter_hit_test_handle(const JiSplitter* sp, int x, int y) {
    if (!sp) return -1;

    for (int i = 0; i < sp->panel_count - 1; i++) {
        JiRect hr = ji_splitter_get_handle_rect(sp, i);
        if (x >= hr.x && x < hr.x + hr.width &&
            y >= hr.y && y < hr.y + hr.height) {
            return i;
        }
    }
    return -1;
}

/* ---- Collapse ---- */

void ji_splitter_toggle_collapse(JiSplitter* sp, int index) {
    if (!sp || index < 0 || index >= sp->panel_count) return;
    if (!sp->panels[index].collapsible) return;

    sp->panels[index].is_collapsed = !sp->panels[index].is_collapsed;
    recalculate_sizes(sp);
}

bool ji_splitter_is_collapsed(const JiSplitter* sp, int index) {
    if (!sp || index < 0 || index >= sp->panel_count) return false;
    return sp->panels[index].is_collapsed;
}

void ji_splitter_set_collapsible(JiSplitter* sp, int index, bool collapsible) {
    if (!sp || index < 0 || index >= sp->panel_count) return;
    sp->panels[index].collapsible = collapsible;
}
