/**
 * JiUI - Dial implementation
 */

#include <jiui/ji_dial.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>

JiDial* ji_dial_new(void) {
    JiDial* dial = (JiDial*)ji_calloc(1, sizeof(JiDial));
    if (!dial) { JI_ERROR_LOG("ji_dial_new: out of memory"); return NULL; }
    dial->minimum = 0;
    dial->maximum = 99;
    dial->value = 0;
    dial->single_step = 1;
    dial->page_step = 10;
    dial->wrapping = false;
    dial->notches_visible = true;
    dial->notch_target = 10;
    return dial;
}

void ji_dial_destroy(JiDial* dial) { if (dial) ji_free(dial); }

void ji_dial_set_value(JiDial* dial, int value) {
    if (!dial) return;
    if (value < dial->minimum) value = dial->wrapping ? dial->maximum : dial->minimum;
    if (value > dial->maximum) value = dial->wrapping ? dial->minimum : dial->maximum;
    dial->value = value;
}

int ji_dial_get_value(const JiDial* dial) { return dial ? dial->value : 0; }

void ji_dial_set_range(JiDial* dial, int minimum, int maximum) {
    if (!dial) return;
    dial->minimum = minimum;
    dial->maximum = maximum;
    if (dial->value < minimum) dial->value = minimum;
    if (dial->value > maximum) dial->value = maximum;
}

void ji_dial_set_wrapping(JiDial* dial, bool wrapping) { if (dial) dial->wrapping = wrapping; }
void ji_dial_set_notches_visible(JiDial* dial, bool visible) { if (dial) dial->notches_visible = visible; }
