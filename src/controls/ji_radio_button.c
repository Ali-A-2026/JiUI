/**
 * JiUI - RadioButton implementation
 */

#include <jiui/ji_radio_button.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

JiRadioButtonGroup* ji_radio_button_group_new(void) {
    JiRadioButtonGroup* g = (JiRadioButtonGroup*)ji_calloc(1, sizeof(JiRadioButtonGroup));
    if (!g) { JI_ERROR_LOG("ji_radio_button_group_new: out of memory"); return NULL; }
    g->button_capacity = 4;
    g->buttons = (JiRadioButton**)ji_alloc(sizeof(JiRadioButton*) * g->button_capacity);
    g->selected_index = -1;
    return g;
}

void ji_radio_button_group_destroy(JiRadioButtonGroup* group) {
    if (!group) return;
    for (int i = 0; i < group->button_count; i++) {
        if (group->buttons[i]) { ji_free(group->buttons[i]->text); ji_free(group->buttons[i]); }
    }
    ji_free(group->buttons);
    ji_free(group);
}

static void grow_buttons(JiRadioButtonGroup* g) {
    if (g->button_count < g->button_capacity) return;
    int new_cap = g->button_capacity * 2;
    JiRadioButton** new_arr = (JiRadioButton**)ji_alloc(sizeof(JiRadioButton*) * new_cap);
    if (!new_arr) return;
    memcpy(new_arr, g->buttons, sizeof(JiRadioButton*) * g->button_count);
    ji_free(g->buttons);
    g->buttons = new_arr;
    g->button_capacity = new_cap;
}

JiRadioButton* ji_radio_button_new(JiRadioButtonGroup* group, const char* text) {
    JiRadioButton* rb = (JiRadioButton*)ji_calloc(1, sizeof(JiRadioButton));
    if (!rb) { JI_ERROR_LOG("ji_radio_button_new: out of memory"); return NULL; }
    if (text) {
        size_t len = strlen(text);
        rb->text = (char*)ji_alloc(len + 1);
        if (rb->text) memcpy(rb->text, text, len + 1);
    }
    rb->group = group;
    if (group) {
        grow_buttons(group);
        group->buttons[group->button_count++] = rb;
        if (group->button_count == 1) { rb->is_checked = true; group->selected_index = 0; }
    }
    return rb;
}

void ji_radio_button_destroy(JiRadioButton* rb) {
    if (!rb) return;
    if (rb->group) {
        for (int i = 0; i < rb->group->button_count; i++) {
            if (rb->group->buttons[i] == rb) {
                for (int j = i; j < rb->group->button_count - 1; j++)
                    rb->group->buttons[j] = rb->group->buttons[j + 1];
                rb->group->button_count--;
                break;
            }
        }
    }
    ji_free(rb->text);
    ji_free(rb);
}

void ji_radio_button_set_checked(JiRadioButton* rb, bool checked) {
    if (!rb || !rb->group) return;
    if (!checked) return; /* cannot uncheck a radio button directly */
    /* Uncheck all others in the group */
    for (int i = 0; i < rb->group->button_count; i++) {
        rb->group->buttons[i]->is_checked = false;
    }
    rb->is_checked = true;
    /* Update selected index */
    for (int i = 0; i < rb->group->button_count; i++) {
        if (rb->group->buttons[i] == rb) { rb->group->selected_index = i; break; }
    }
}

bool ji_radio_button_is_checked(const JiRadioButton* rb) { return rb ? rb->is_checked : false; }
int ji_radio_button_group_get_selected(const JiRadioButtonGroup* group) { return group ? group->selected_index : -1; }

void ji_radio_button_group_set_selected(JiRadioButtonGroup* group, int index) {
    if (!group || index < 0 || index >= group->button_count) return;
    ji_radio_button_set_checked(group->buttons[index], true);
}

int ji_radio_button_group_count(const JiRadioButtonGroup* group) { return group ? group->button_count : 0; }
