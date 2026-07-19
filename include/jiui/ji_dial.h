/**
 * JiUI - Dial widget header
 */

#ifndef JIUI_DIAL_H
#define JIUI_DIAL_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiDial {
    JiControl  control;
    int        value;
    int        minimum;
    int        maximum;
    int        single_step;
    int        page_step;
    bool       wrapping;
    bool       notches_visible;
    int        notch_target;    /* target number of notches (default 10) */
} JiDial;

JI_API JiDial* ji_dial_new(void);
JI_API void ji_dial_destroy(JiDial* dial);
JI_API void ji_dial_set_value(JiDial* dial, int value);
JI_API int ji_dial_get_value(const JiDial* dial);
JI_API void ji_dial_set_range(JiDial* dial, int minimum, int maximum);
JI_API void ji_dial_set_wrapping(JiDial* dial, bool wrapping);
JI_API void ji_dial_set_notches_visible(JiDial* dial, bool visible);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_DIAL_H */
