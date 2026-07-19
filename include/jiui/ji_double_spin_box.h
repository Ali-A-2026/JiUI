/**
 * JiUI - DoubleSpinBox widget header
 */

#ifndef JIUI_DOUBLE_SPIN_BOX_H
#define JIUI_DOUBLE_SPIN_BOX_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiDoubleSpinBox {
    JiControl  control;
    double     value;
    double     minimum;
    double     maximum;
    double     single_step;
    int        decimals;       /* number of decimal places (default 2) */
    char*      prefix;
    char*      suffix;
    bool       wrapping;
} JiDoubleSpinBox;

JI_API JiDoubleSpinBox* ji_double_spin_box_new(void);
JI_API void ji_double_spin_box_destroy(JiDoubleSpinBox* sb);
JI_API void ji_double_spin_box_set_value(JiDoubleSpinBox* sb, double value);
JI_API double ji_double_spin_box_get_value(const JiDoubleSpinBox* sb);
JI_API void ji_double_spin_box_set_range(JiDoubleSpinBox* sb, double minimum, double maximum);
JI_API void ji_double_spin_box_set_single_step(JiDoubleSpinBox* sb, double step);
JI_API void ji_double_spin_box_set_decimals(JiDoubleSpinBox* sb, int decimals);
JI_API void ji_double_spin_box_set_prefix(JiDoubleSpinBox* sb, const char* prefix);
JI_API void ji_double_spin_box_set_suffix(JiDoubleSpinBox* sb, const char* suffix);
JI_API void ji_double_spin_box_step_up(JiDoubleSpinBox* sb);
JI_API void ji_double_spin_box_step_down(JiDoubleSpinBox* sb);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_DOUBLE_SPIN_BOX_H */
