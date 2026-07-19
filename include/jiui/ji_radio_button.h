/**
 * JiUI - RadioButton widget header
 */

#ifndef JIUI_RADIO_BUTTON_H
#define JIUI_RADIO_BUTTON_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiRadioButtonGroup JiRadioButtonGroup;

typedef struct JiRadioButton {
    JiControl             control;       /* must be first */
    bool                  is_checked;
    JiRadioButtonGroup*   group;
    char*                 text;
} JiRadioButton;

typedef struct JiRadioButtonGroup {
    JiRadioButton**  buttons;
    int              button_count;
    int              button_capacity;
    int              selected_index;
} JiRadioButtonGroup;

/** Create a new radio button group. */
JI_API JiRadioButtonGroup* ji_radio_button_group_new(void);

/** Destroy a radio button group. */
JI_API void ji_radio_button_group_destroy(JiRadioButtonGroup* group);

/** Create a new radio button in the given group. */
JI_API JiRadioButton* ji_radio_button_new(JiRadioButtonGroup* group, const char* text);

/** Destroy a radio button. */
JI_API void ji_radio_button_destroy(JiRadioButton* rb);

/** Set the checked state. */
JI_API void ji_radio_button_set_checked(JiRadioButton* rb, bool checked);

/** Check if the radio button is checked. */
JI_API bool ji_radio_button_is_checked(const JiRadioButton* rb);

/** Get the selected index in the group (-1 if none). */
JI_API int ji_radio_button_group_get_selected(const JiRadioButtonGroup* group);

/** Set the selected index in the group. */
JI_API void ji_radio_button_group_set_selected(JiRadioButtonGroup* group, int index);

/** Get the number of buttons in the group. */
JI_API int ji_radio_button_group_count(const JiRadioButtonGroup* group);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_RADIO_BUTTON_H */
