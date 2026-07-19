/**
 * JiUI - StackedWidget implementation
 */

#include <jiui/ji_stacked_widget.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

static void grow_pages(JiStackedWidget* sw) {
    if (sw->page_count < sw->page_capacity) return;
    int new_cap = sw->page_capacity * 2;
    JiControl** new_arr = (JiControl**)ji_alloc(sizeof(JiControl*) * new_cap);
    if (!new_arr) return;
    memcpy(new_arr, sw->pages, sizeof(JiControl*) * sw->page_count);
    ji_free(sw->pages);
    sw->pages = new_arr;
    sw->page_capacity = new_cap;
}

JiStackedWidget* ji_stacked_widget_new(void) {
    JiStackedWidget* sw = (JiStackedWidget*)ji_calloc(1, sizeof(JiStackedWidget));
    if (!sw) { JI_ERROR_LOG("ji_stacked_widget_new: out of memory"); return NULL; }
    sw->page_capacity = 4;
    sw->pages = (JiControl**)ji_alloc(sizeof(JiControl*) * sw->page_capacity);
    sw->current_index = 0;
    return sw;
}

void ji_stacked_widget_destroy(JiStackedWidget* sw) {
    if (!sw) return;
    ji_free(sw->pages);
    ji_free(sw);
}

int ji_stacked_widget_add_page(JiStackedWidget* sw, JiControl* page) {
    if (!sw) return -1;
    grow_pages(sw);
    sw->pages[sw->page_count++] = page;
    return sw->page_count - 1;
}

void ji_stacked_widget_insert_page(JiStackedWidget* sw, int index, JiControl* page) {
    if (!sw || index < 0 || index > sw->page_count) return;
    grow_pages(sw);
    for (int i = sw->page_count; i > index; i--) sw->pages[i] = sw->pages[i - 1];
    sw->pages[index] = page;
    sw->page_count++;
    if (sw->current_index >= index) sw->current_index++;
}

void ji_stacked_widget_remove_page(JiStackedWidget* sw, int index) {
    if (!sw || index < 0 || index >= sw->page_count) return;
    for (int i = index; i < sw->page_count - 1; i++) sw->pages[i] = sw->pages[i + 1];
    sw->page_count--;
    if (sw->current_index >= sw->page_count) sw->current_index = sw->page_count > 0 ? sw->page_count - 1 : 0;
}

int ji_stacked_widget_count(const JiStackedWidget* sw) { return sw ? sw->page_count : 0; }
void ji_stacked_widget_set_current_index(JiStackedWidget* sw, int index) { if (sw && index >= 0 && index < sw->page_count) sw->current_index = index; }
int ji_stacked_widget_get_current_index(const JiStackedWidget* sw) { return sw ? sw->current_index : 0; }
JiControl* ji_stacked_widget_get_current_page(const JiStackedWidget* sw) { return (sw && sw->page_count > 0) ? sw->pages[sw->current_index] : NULL; }
JiControl* ji_stacked_widget_get_page(const JiStackedWidget* sw, int index) { return (sw && index >= 0 && index < sw->page_count) ? sw->pages[index] : NULL; }

int ji_stacked_widget_index_of(const JiStackedWidget* sw, JiControl* page) {
    if (!sw) return -1;
    for (int i = 0; i < sw->page_count; i++) { if (sw->pages[i] == page) return i; }
    return -1;
}
