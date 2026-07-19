/**
 * JiUI - SpinBox widget header
 */

#ifndef JIUI_SPIN_BOX_H
#define JIUI_SPIN_BOX_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiSpinBox {
    JiControl  control;        /* must be first */
    int        value;
    int        minimum;
    int        maximum;
    int        single_step;    /* step when clicking arrows (default 1) */
    char*      prefix;
    char*      suffix;
    bool       wrapping;       /* wrap around at min/max */
    int        button_width;   /* width of up/down buttons (default 16) */
} JiSpinBox;

JI_API JiSpinBox* ji_spin_box_new(void);
JI_API void ji_spin_box_destroy(JiSpinBox* sb);
JI_API void ji_spin_box_set_value(JiSpinBox* sb, int value);
JI_API int ji_spin_box_get_value(const JiSpinBox* sb);
JI_API void ji_spin_box_set_range(JiSpinBox* sb, int minimum, int maximum);
JI_API void ji_spin_box_set_single_step(JiSpinBox* sb, int step);
JI_API void ji_spin_box_set_prefix(JiSpinBox* sb, const char* prefix);
JI_API void ji_spin_box_set_suffix(JiSpinBox* sb, const char* suffix);
JI_API void ji_spin_box_set_wrapping(JiSpinBox* sb, bool wrapping);
JI_API void ji_spin_box_step_up(JiSpinBox* sb);
JI_API void ji_spin_box_step_down(JiSpinBox* sb);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_SPIN_BOX_H */
