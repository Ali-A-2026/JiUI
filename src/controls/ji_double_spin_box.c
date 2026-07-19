/**
 * JiUI - DoubleSpinBox implementation
 */

#include <jiui/ji_double_spin_box.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

JiDoubleSpinBox* ji_double_spin_box_new(void) {
    JiDoubleSpinBox* sb = (JiDoubleSpinBox*)ji_calloc(1, sizeof(JiDoubleSpinBox));
    if (!sb) { JI_ERROR_LOG("ji_double_spin_box_new: out of memory"); return NULL; }
    sb->minimum = 0.0;
    sb->maximum = 99.99;
    sb->value = 0.0;
    sb->single_step = 1.0;
    sb->decimals = 2;
    sb->wrapping = false;
    return sb;
}

void ji_double_spin_box_destroy(JiDoubleSpinBox* sb) {
    if (!sb) return;
    ji_free(sb->prefix);
    ji_free(sb->suffix);
    ji_free(sb);
}

void ji_double_spin_box_set_value(JiDoubleSpinBox* sb, double value) {
    if (!sb) return;
    if (value < sb->minimum) value = sb->wrapping ? sb->maximum : sb->minimum;
    if (value > sb->maximum) value = sb->wrapping ? sb->minimum : sb->maximum;
    sb->value = value;
}

double ji_double_spin_box_get_value(const JiDoubleSpinBox* sb) { return sb ? sb->value : 0.0; }

void ji_double_spin_box_set_range(JiDoubleSpinBox* sb, double minimum, double maximum) {
    if (!sb) return;
    sb->minimum = minimum;
    sb->maximum = maximum;
    if (sb->value < minimum) sb->value = minimum;
    if (sb->value > maximum) sb->value = maximum;
}

void ji_double_spin_box_set_single_step(JiDoubleSpinBox* sb, double step) { if (sb && step > 0) sb->single_step = step; }
void ji_double_spin_box_set_decimals(JiDoubleSpinBox* sb, int decimals) { if (sb) sb->decimals = decimals > 0 ? decimals : 0; }

void ji_double_spin_box_set_prefix(JiDoubleSpinBox* sb, const char* prefix) {
    if (!sb) return;
    ji_free(sb->prefix);
    if (prefix) { size_t len = strlen(prefix); sb->prefix = (char*)ji_alloc(len+1); if (sb->prefix) memcpy(sb->prefix, prefix, len+1); }
    else sb->prefix = NULL;
}

void ji_double_spin_box_set_suffix(JiDoubleSpinBox* sb, const char* suffix) {
    if (!sb) return;
    ji_free(sb->suffix);
    if (suffix) { size_t len = strlen(suffix); sb->suffix = (char*)ji_alloc(len+1); if (sb->suffix) memcpy(sb->suffix, suffix, len+1); }
    else sb->suffix = NULL;
}

void ji_double_spin_box_step_up(JiDoubleSpinBox* sb) {
    if (!sb) return;
    double new_val = sb->value + sb->single_step;
    if (new_val > sb->maximum) new_val = sb->wrapping ? sb->minimum : sb->maximum;
    sb->value = new_val;
}

void ji_double_spin_box_step_down(JiDoubleSpinBox* sb) {
    if (!sb) return;
    double new_val = sb->value - sb->single_step;
    if (new_val < sb->minimum) new_val = sb->wrapping ? sb->maximum : sb->minimum;
    sb->value = new_val;
}
