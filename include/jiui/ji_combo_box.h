/**
 * JiUI - ComboBox widget header
 */

#ifndef JIUI_COMBO_BOX_H
#define JIUI_COMBO_BOX_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiComboBoxItem {
    char*   text;
    void*   user_data;
} JiComboBoxItem;

typedef struct JiComboBox {
    JiControl          control;
    JiComboBoxItem*    items;
    int                item_count;
    int                item_capacity;
    int                current_index;
    bool               is_editable;
    bool               is_popup_visible;
    int                max_visible_items;  /* default 10 */
    int                dropdown_height;
} JiComboBox;

JI_API JiComboBox* ji_combo_box_new(void);
JI_API void ji_combo_box_destroy(JiComboBox* cb);
JI_API int ji_combo_box_add_item(JiComboBox* cb, const char* text);
JI_API void ji_combo_box_insert_item(JiComboBox* cb, int index, const char* text);
JI_API void ji_combo_box_remove_item(JiComboBox* cb, int index);
JI_API int ji_combo_box_count(const JiComboBox* cb);
JI_API void ji_combo_box_set_current_index(JiComboBox* cb, int index);
JI_API int ji_combo_box_get_current_index(const JiComboBox* cb);
JI_API const char* ji_combo_box_get_current_text(const JiComboBox* cb);
JI_API const char* ji_combo_box_get_item_text(const JiComboBox* cb, int index);
JI_API void ji_combo_box_set_editable(JiComboBox* cb, bool editable);
JI_API bool ji_combo_box_is_editable(const JiComboBox* cb);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_COMBO_BOX_H */
