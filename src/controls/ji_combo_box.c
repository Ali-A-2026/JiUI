/**
 * JiUI - ComboBox implementation
 */

#include <jiui/ji_combo_box.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

static void grow_items(JiComboBox* cb) {
    if (cb->item_count < cb->item_capacity) return;
    int new_cap = cb->item_capacity * 2;
    JiComboBoxItem* new_arr = (JiComboBoxItem*)ji_alloc(sizeof(JiComboBoxItem) * new_cap);
    if (!new_arr) return;
    memcpy(new_arr, cb->items, sizeof(JiComboBoxItem) * cb->item_count);
    ji_free(cb->items);
    cb->items = new_arr;
    cb->item_capacity = new_cap;
}

JiComboBox* ji_combo_box_new(void) {
    JiComboBox* cb = (JiComboBox*)ji_calloc(1, sizeof(JiComboBox));
    if (!cb) { JI_ERROR_LOG("ji_combo_box_new: out of memory"); return NULL; }
    cb->item_capacity = 8;
    cb->items = (JiComboBoxItem*)ji_alloc(sizeof(JiComboBoxItem) * cb->item_capacity);
    cb->current_index = -1;
    cb->is_editable = false;
    cb->is_popup_visible = false;
    cb->max_visible_items = 10;
    return cb;
}

void ji_combo_box_destroy(JiComboBox* cb) {
    if (!cb) return;
    for (int i = 0; i < cb->item_count; i++) ji_free(cb->items[i].text);
    ji_free(cb->items);
    ji_free(cb);
}

int ji_combo_box_add_item(JiComboBox* cb, const char* text) {
    if (!cb) return -1;
    grow_items(cb);
    JiComboBoxItem* item = &cb->items[cb->item_count];
    memset(item, 0, sizeof(*item));
    if (text) { size_t len = strlen(text); item->text = (char*)ji_alloc(len+1); if (item->text) memcpy(item->text, text, len+1); }
    item->user_data = NULL;
    if (cb->current_index < 0) cb->current_index = 0;
    return cb->item_count++;
}

void ji_combo_box_insert_item(JiComboBox* cb, int index, const char* text) {
    if (!cb || index < 0 || index > cb->item_count) return;
    grow_items(cb);
    for (int i = cb->item_count; i > index; i--) cb->items[i] = cb->items[i-1];
    JiComboBoxItem* item = &cb->items[index];
    memset(item, 0, sizeof(*item));
    if (text) { size_t len = strlen(text); item->text = (char*)ji_alloc(len+1); if (item->text) memcpy(item->text, text, len+1); }
    if (cb->current_index >= index) cb->current_index++;
    cb->item_count++;
}

void ji_combo_box_remove_item(JiComboBox* cb, int index) {
    if (!cb || index < 0 || index >= cb->item_count) return;
    ji_free(cb->items[index].text);
    for (int i = index; i < cb->item_count - 1; i++) cb->items[i] = cb->items[i+1];
    cb->item_count--;
    if (cb->current_index >= cb->item_count) cb->current_index = cb->item_count > 0 ? cb->item_count - 1 : -1;
}

int ji_combo_box_count(const JiComboBox* cb) { return cb ? cb->item_count : 0; }
void ji_combo_box_set_current_index(JiComboBox* cb, int index) { if (cb && index >= -1 && index < cb->item_count) cb->current_index = index; }
int ji_combo_box_get_current_index(const JiComboBox* cb) { return cb ? cb->current_index : -1; }

const char* ji_combo_box_get_current_text(const JiComboBox* cb) {
    if (!cb || cb->current_index < 0 || cb->current_index >= cb->item_count) return NULL;
    return cb->items[cb->current_index].text;
}

const char* ji_combo_box_get_item_text(const JiComboBox* cb, int index) {
    if (!cb || index < 0 || index >= cb->item_count) return NULL;
    return cb->items[index].text;
}

void ji_combo_box_set_editable(JiComboBox* cb, bool editable) { if (cb) cb->is_editable = editable; }
bool ji_combo_box_is_editable(const JiComboBox* cb) { return cb ? cb->is_editable : false; }
