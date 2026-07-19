/**
 * JiUI - SpinBox implementation
 */

#include <jiui/ji_spin_box.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

JiSpinBox* ji_spin_box_new(void) {
    JiSpinBox* sb = (JiSpinBox*)ji_calloc(1, sizeof(JiSpinBox));
    if (!sb) { JI_ERROR_LOG("ji_spin_box_new: out of memory"); return NULL; }
    sb->minimum = 0;
    sb->maximum = 99;
    sb->value = 0;
    sb->single_step = 1;
    sb->wrapping = false;
    sb->button_width = 16;
    return sb;
}

void ji_spin_box_destroy(JiSpinBox* sb) {
    if (!sb) return;
    ji_free(sb->prefix);
    ji_free(sb->suffix);
    ji_free(sb);
}

void ji_spin_box_set_value(JiSpinBox* sb, int value) {
    if (!sb) return;
    if (value < sb->minimum) value = sb->wrapping ? sb->maximum : sb->minimum;
    if (value > sb->maximum) value = sb->wrapping ? sb->minimum : sb->maximum;
    sb->value = value;
}

int ji_spin_box_get_value(const JiSpinBox* sb) { return sb ? sb->value : 0; }

void ji_spin_box_set_range(JiSpinBox* sb, int minimum, int maximum) {
    if (!sb) return;
    sb->minimum = minimum;
    sb->maximum = maximum;
    if (sb->value < minimum) sb->value = minimum;
    if (sb->value > maximum) sb->value = maximum;
}

void ji_spin_box_set_single_step(JiSpinBox* sb, int step) { if (sb) sb->single_step = step > 0 ? step : 1; }

void ji_spin_box_set_prefix(JiSpinBox* sb, const char* prefix) {
    if (!sb) return;
    ji_free(sb->prefix);
    if (prefix) { size_t len = strlen(prefix); sb->prefix = (char*)ji_alloc(len+1); if (sb->prefix) memcpy(sb->prefix, prefix, len+1); }
    else sb->prefix = NULL;
}

void ji_spin_box_set_suffix(JiSpinBox* sb, const char* suffix) {
    if (!sb) return;
    ji_free(sb->suffix);
    if (suffix) { size_t len = strlen(suffix); sb->suffix = (char*)ji_alloc(len+1); if (sb->suffix) memcpy(sb->suffix, suffix, len+1); }
    else sb->suffix = NULL;
}

void ji_spin_box_set_wrapping(JiSpinBox* sb, bool wrapping) { if (sb) sb->wrapping = wrapping; }

void ji_spin_box_step_up(JiSpinBox* sb) {
    if (!sb) return;
    int new_val = sb->value + sb->single_step;
    if (new_val > sb->maximum) new_val = sb->wrapping ? sb->minimum : sb->maximum;
    sb->value = new_val;
}

void ji_spin_box_step_down(JiSpinBox* sb) {
    if (!sb) return;
    int new_val = sb->value - sb->single_step;
    if (new_val < sb->minimum) new_val = sb->wrapping ? sb->maximum : sb->minimum;
    sb->value = new_val;
}
