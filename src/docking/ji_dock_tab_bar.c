/**
 * JiUI - Dock Tab Bar implementation
 */

#include <jiui/ji_dock_tab_bar.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

static void grow_tabs(JiDockTabBar* bar) {
    if (bar->tab_count < bar->tab_capacity) return;
    int new_cap = bar->tab_capacity * 2;
    JiDockTabBarTab* new_arr = (JiDockTabBarTab*)ji_alloc(sizeof(JiDockTabBarTab) * new_cap);
    if (!new_arr) return;
    memcpy(new_arr, bar->tabs, sizeof(JiDockTabBarTab) * bar->tab_count);
    ji_free(bar->tabs);
    bar->tabs = new_arr;
    bar->tab_capacity = new_cap;
}

JiDockTabBar* ji_dock_tab_bar_new(void) {
    JiDockTabBar* bar = (JiDockTabBar*)ji_calloc(1, sizeof(JiDockTabBar));
    if (!bar) { JI_ERROR_LOG("ji_dock_tab_bar_new: out of memory"); return NULL; }
    bar->tab_capacity = 4;
    bar->tabs = (JiDockTabBarTab*)ji_alloc(sizeof(JiDockTabBarTab) * bar->tab_capacity);
    bar->current_index = 0;
    bar->tab_height = 24;
    bar->tabs_closable = true;
    bar->tabs_movable = true;
    bar->drag_index = -1;
    bar->hover_index = -1;
    return bar;
}

void ji_dock_tab_bar_destroy(JiDockTabBar* bar) {
    if (!bar) return;
    for (int i = 0; i < bar->tab_count; i++) ji_free(bar->tabs[i].label);
    ji_free(bar->tabs);
    ji_free(bar);
}

int ji_dock_tab_bar_add_tab(JiDockTabBar* bar, const char* label, void* widget) {
    if (!bar) return -1;
    grow_tabs(bar);
    JiDockTabBarTab* tab = &bar->tabs[bar->tab_count];
    memset(tab, 0, sizeof(*tab));
    if (label) {
        size_t len = strlen(label);
        tab->label = (char*)ji_alloc(len + 1);
        if (tab->label) memcpy(tab->label, label, len + 1);
    }
    tab->widget = widget;
    tab->is_closable = bar->tabs_closable;
    tab->is_enabled = true;
    return bar->tab_count++;
}

void ji_dock_tab_bar_remove_tab(JiDockTabBar* bar, int index) {
    if (!bar || index < 0 || index >= bar->tab_count) return;
    ji_free(bar->tabs[index].label);
    for (int i = index; i < bar->tab_count - 1; i++) bar->tabs[i] = bar->tabs[i + 1];
    bar->tab_count--;
    if (bar->current_index >= bar->tab_count) bar->current_index = bar->tab_count > 0 ? bar->tab_count - 1 : 0;
}

int ji_dock_tab_bar_count(const JiDockTabBar* bar) { return bar ? bar->tab_count : 0; }
void ji_dock_tab_bar_set_current(JiDockTabBar* bar, int index) { if (bar && index >= 0 && index < bar->tab_count) bar->current_index = index; }
int ji_dock_tab_bar_get_current(const JiDockTabBar* bar) { return bar ? bar->current_index : 0; }

void ji_dock_tab_bar_set_tab_label(JiDockTabBar* bar, int index, const char* label) {
    if (!bar || index < 0 || index >= bar->tab_count) return;
    ji_free(bar->tabs[index].label);
    if (label) {
        size_t len = strlen(label);
        bar->tabs[index].label = (char*)ji_alloc(len + 1);
        if (bar->tabs[index].label) memcpy(bar->tabs[index].label, label, len + 1);
    } else bar->tabs[index].label = NULL;
}

const char* ji_dock_tab_bar_get_tab_label(const JiDockTabBar* bar, int index) {
    return (bar && index >= 0 && index < bar->tab_count) ? bar->tabs[index].label : NULL;
}
