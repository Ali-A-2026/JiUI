/**
 * JiUI - TextEdit widget header
 */

#ifndef JIUI_TEXT_EDIT_H
#define JIUI_TEXT_EDIT_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiTextRange {
    int start;
    int end;
} JiTextRange;

typedef struct JiTextEdit {
    JiControl    control;
    char*        text;
    int          text_capacity;
    int          text_length;
    JiTextRange  selection;
    int          cursor_pos;
    bool         is_read_only;
    bool         is_rich_text;
    int          tab_width;       /* default 4 */
    int          line_count;
    bool         line_wrap;
} JiTextEdit;

JI_API JiTextEdit* ji_text_edit_new(void);
JI_API void ji_text_edit_destroy(JiTextEdit* te);
JI_API void ji_text_edit_set_text(JiTextEdit* te, const char* text);
JI_API const char* ji_text_edit_get_text(const JiTextEdit* te);
JI_API void ji_text_edit_set_read_only(JiTextEdit* te, bool read_only);
JI_API bool ji_text_edit_is_read_only(const JiTextEdit* te);
JI_API void ji_text_edit_set_cursor_position(JiTextEdit* te, int pos);
JI_API int ji_text_edit_get_cursor_position(const JiTextEdit* te);
JI_API void ji_text_edit_select_all(JiTextEdit* te);
JI_API void ji_text_edit_set_selection(JiTextEdit* te, int start, int end);
JI_API void ji_text_edit_clear_selection(JiTextEdit* te);
JI_API void ji_text_edit_insert_text(JiTextEdit* te, const char* text);
JI_API void ji_text_edit_append_text(JiTextEdit* te, const char* text);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_TEXT_EDIT_H */
