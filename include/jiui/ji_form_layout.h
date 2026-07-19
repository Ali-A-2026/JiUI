/**
 * JiUI - FormLayout widget header
 */

#ifndef JIUI_FORM_LAYOUT_H
#define JIUI_FORM_LAYOUT_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JiFormLayoutFieldGrowthPolicy {
    JI_FORM_FIELDS_STAY_AT_SIZE_HINT = 0,
    JI_FORM_FIELDS_EXPANDING = 1,
    JI_FORM_FIELDS_ALL_NON_FIXED = 2
} JiFormLayoutFieldGrowthPolicy;

typedef struct JiFormLayoutRow {
    char*       label_text;
    JiControl*  label_control;
    JiControl*  field_control;
} JiFormLayoutRow;

typedef struct JiFormLayout {
    JiControl                      control;
    JiFormLayoutRow*               rows;
    int                            row_count;
    int                            row_capacity;
    int                            spacing;
    int                            horizontal_spacing;
    int                            vertical_spacing;
    int                            margin_left;
    int                            margin_top;
    int                            margin_right;
    int                            margin_bottom;
    JiFormLayoutFieldGrowthPolicy field_growth;
} JiFormLayout;

JI_API JiFormLayout* ji_form_layout_new(void);
JI_API void ji_form_layout_destroy(JiFormLayout* layout);
JI_API int ji_form_layout_add_row(JiFormLayout* layout, const char* label, JiControl* field);
JI_API void ji_form_layout_insert_row(JiFormLayout* layout, int index, const char* label, JiControl* field);
JI_API void ji_form_layout_remove_row(JiFormLayout* layout, int index);
JI_API int ji_form_layout_row_count(const JiFormLayout* layout);
JI_API void ji_form_layout_set_spacing(JiFormLayout* layout, int spacing);
JI_API void ji_form_layout_set_field_growth(JiFormLayout* layout, JiFormLayoutFieldGrowthPolicy policy);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_FORM_LAYOUT_H */
