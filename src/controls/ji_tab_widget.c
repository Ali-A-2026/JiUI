/**
 * JiUI - TabWidget implementation
 */

#include <jiui/ji_tab_widget.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

static void grow_pages(JiTabWidget* tw) {
    if (tw->page_count < tw->page_capacity) return;
    int new_cap = tw->page_capacity * 2;
    JiTabWidgetPage* new_arr = (JiTabWidgetPage*)ji_alloc(sizeof(JiTabWidgetPage) * new_cap);
    if (!new_arr) return;
    memcpy(new_arr, tw->pages, sizeof(JiTabWidgetPage) * tw->page_count);
    ji_free(tw->pages);
    tw->pages = new_arr;
    tw->page_capacity = new_cap;
}

JiTabWidget* ji_tab_widget_new(void) {
    JiTabWidget* tw = (JiTabWidget*)ji_calloc(1, sizeof(JiTabWidget));
    if (!tw) { JI_ERROR_LOG("ji_tab_widget_new: out of memory"); return NULL; }
    tw->page_capacity = 4;
    tw->pages = (JiTabWidgetPage*)ji_alloc(sizeof(JiTabWidgetPage) * tw->page_capacity);
    tw->current_index = 0;
    tw->tab_height = 28;
    tw->tabs_closable = false;
    tw->tabs_movable = false;
    return tw;
}

void ji_tab_widget_destroy(JiTabWidget* tw) {
    if (!tw) return;
    for (int i = 0; i < tw->page_count; i++) ji_free(tw->pages[i].label);
    ji_free(tw->pages);
    ji_free(tw);
}

int ji_tab_widget_add_tab(JiTabWidget* tw, const char* label, JiControl* content) {
    if (!tw) return -1;
    grow_pages(tw);
    JiTabWidgetPage* p = &tw->pages[tw->page_count];
    memset(p, 0, sizeof(*p));
    if (label) {
        size_t len = strlen(label);
        p->label = (char*)ji_alloc(len + 1);
        if (p->label) memcpy(p->label, label, len + 1);
    }
    p->content = content;
    p->is_closable = tw->tabs_closable;
    p->is_enabled = true;
    return tw->page_count++;
}

void ji_tab_widget_insert_tab(JiTabWidget* tw, int index, const char* label, JiControl* content) {
    if (!tw || index < 0 || index > tw->page_count) return;
    grow_pages(tw);
    for (int i = tw->page_count; i > index; i--) tw->pages[i] = tw->pages[i - 1];
    JiTabWidgetPage* p = &tw->pages[index];
    memset(p, 0, sizeof(*p));
    if (label) {
        size_t len = strlen(label);
        p->label = (char*)ji_alloc(len + 1);
        if (p->label) memcpy(p->label, label, len + 1);
    }
    p->content = content;
    p->is_closable = tw->tabs_closable;
    p->is_enabled = true;
    tw->page_count++;
    if (tw->current_index >= index) tw->current_index++;
}

void ji_tab_widget_remove_tab(JiTabWidget* tw, int index) {
    if (!tw || index < 0 || index >= tw->page_count) return;
    ji_free(tw->pages[index].label);
    for (int i = index; i < tw->page_count - 1; i++) tw->pages[i] = tw->pages[i + 1];
    tw->page_count--;
    if (tw->current_index >= tw->page_count) tw->current_index = tw->page_count > 0 ? tw->page_count - 1 : 0;
}

int ji_tab_widget_count(const JiTabWidget* tw) { return tw ? tw->page_count : 0; }
void ji_tab_widget_set_current_index(JiTabWidget* tw, int index) { if (tw && index >= 0 && index < tw->page_count) tw->current_index = index; }
int ji_tab_widget_get_current_index(const JiTabWidget* tw) { return tw ? tw->current_index : 0; }
JiControl* ji_tab_widget_get_content(const JiTabWidget* tw, int index) { return (tw && index >= 0 && index < tw->page_count) ? tw->pages[index].content : NULL; }

void ji_tab_widget_set_tab_label(JiTabWidget* tw, int index, const char* label) {
    if (!tw || index < 0 || index >= tw->page_count) return;
    ji_free(tw->pages[index].label);
    if (label) {
        size_t len = strlen(label);
        tw->pages[index].label = (char*)ji_alloc(len + 1);
        if (tw->pages[index].label) memcpy(tw->pages[index].label, label, len + 1);
    } else { tw->pages[index].label = NULL; }
}

const char* ji_tab_widget_get_tab_label(const JiTabWidget* tw, int index) {
    return (tw && index >= 0 && index < tw->page_count) ? tw->pages[index].label : NULL;
}

void ji_tab_widget_set_tab_closable(JiTabWidget* tw, int index, bool closable) {
    if (tw && index >= 0 && index < tw->page_count) tw->pages[index].is_closable = closable;
}

void ji_tab_widget_set_tab_enabled(JiTabWidget* tw, int index, bool enabled) {
    if (tw && index >= 0 && index < tw->page_count) tw->pages[index].is_enabled = enabled;
}
