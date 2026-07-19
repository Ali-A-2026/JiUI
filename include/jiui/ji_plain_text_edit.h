/**
 * JiUI - PlainTextEdit widget header
 */

#ifndef JIUI_PLAIN_TEXT_EDIT_H
#define JIUI_PLAIN_TEXT_EDIT_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiPlainTextEdit {
    JiControl    control;
    char*        text;
    int          text_capacity;
    int          text_length;
    int          cursor_pos;
    bool         is_read_only;
    int          tab_width;
    bool         line_wrap;
    int          max_line_length;  /* 0 = unlimited */
} JiPlainTextEdit;

JI_API JiPlainTextEdit* ji_plain_text_edit_new(void);
JI_API void ji_plain_text_edit_destroy(JiPlainTextEdit* te);
JI_API void ji_plain_text_edit_set_text(JiPlainTextEdit* te, const char* text);
JI_API const char* ji_plain_text_edit_get_text(const JiPlainTextEdit* te);
JI_API void ji_plain_text_edit_set_read_only(JiPlainTextEdit* te, bool read_only);
JI_API void ji_plain_text_edit_append_text(JiPlainTextEdit* te, const char* text);
JI_API void ji_plain_text_edit_set_line_wrap(JiPlainTextEdit* te, bool wrap);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_PLAIN_TEXT_EDIT_H */
