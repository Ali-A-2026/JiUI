/**
 * JiUI - FormLayout implementation
 */

#include <jiui/ji_form_layout.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

static void grow_rows(JiFormLayout* layout) {
    if (layout->row_count < layout->row_capacity) return;
    int new_cap = layout->row_capacity * 2;
    JiFormLayoutRow* new_arr = (JiFormLayoutRow*)ji_alloc(sizeof(JiFormLayoutRow) * new_cap);
    if (!new_arr) return;
    memcpy(new_arr, layout->rows, sizeof(JiFormLayoutRow) * layout->row_count);
    ji_free(layout->rows);
    layout->rows = new_arr;
    layout->row_capacity = new_cap;
}

JiFormLayout* ji_form_layout_new(void) {
    JiFormLayout* layout = (JiFormLayout*)ji_calloc(1, sizeof(JiFormLayout));
    if (!layout) { JI_ERROR_LOG("ji_form_layout_new: out of memory"); return NULL; }
    layout->row_capacity = 8;
    layout->rows = (JiFormLayoutRow*)ji_alloc(sizeof(JiFormLayoutRow) * layout->row_capacity);
    layout->spacing = 6;
    layout->horizontal_spacing = 6;
    layout->vertical_spacing = 6;
    layout->margin_left = 0;
    layout->margin_top = 0;
    layout->margin_right = 0;
    layout->margin_bottom = 0;
    layout->field_growth = JI_FORM_FIELDS_EXPANDING;
    return layout;
}

void ji_form_layout_destroy(JiFormLayout* layout) {
    if (!layout) return;
    for (int i = 0; i < layout->row_count; i++) ji_free(layout->rows[i].label_text);
    ji_free(layout->rows);
    ji_free(layout);
}

int ji_form_layout_add_row(JiFormLayout* layout, const char* label, JiControl* field) {
    if (!layout) return -1;
    grow_rows(layout);
    JiFormLayoutRow* row = &layout->rows[layout->row_count];
    memset(row, 0, sizeof(*row));
    if (label) {
        size_t len = strlen(label);
        row->label_text = (char*)ji_alloc(len + 1);
        if (row->label_text) memcpy(row->label_text, label, len + 1);
    }
    row->field_control = field;
    return layout->row_count++;
}

void ji_form_layout_insert_row(JiFormLayout* layout, int index, const char* label, JiControl* field) {
    if (!layout || index < 0 || index > layout->row_count) return;
    grow_rows(layout);
    for (int i = layout->row_count; i > index; i--) layout->rows[i] = layout->rows[i - 1];
    JiFormLayoutRow* row = &layout->rows[index];
    memset(row, 0, sizeof(*row));
    if (label) {
        size_t len = strlen(label);
        row->label_text = (char*)ji_alloc(len + 1);
        if (row->label_text) memcpy(row->label_text, label, len + 1);
    }
    row->field_control = field;
    layout->row_count++;
}

void ji_form_layout_remove_row(JiFormLayout* layout, int index) {
    if (!layout || index < 0 || index >= layout->row_count) return;
    ji_free(layout->rows[index].label_text);
    for (int i = index; i < layout->row_count - 1; i++) layout->rows[i] = layout->rows[i + 1];
    layout->row_count--;
}

int ji_form_layout_row_count(const JiFormLayout* layout) { return layout ? layout->row_count : 0; }
void ji_form_layout_set_spacing(JiFormLayout* layout, int spacing) { if (layout) { layout->spacing = spacing; layout->horizontal_spacing = spacing; layout->vertical_spacing = spacing; } }
void ji_form_layout_set_field_growth(JiFormLayout* layout, JiFormLayoutFieldGrowthPolicy policy) { if (layout) layout->field_growth = policy; }
