/**
 * JiUI - TextEdit implementation
 */

#include <jiui/ji_text_edit.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

JiTextEdit* ji_text_edit_new(void) {
    JiTextEdit* te = (JiTextEdit*)ji_calloc(1, sizeof(JiTextEdit));
    if (!te) { JI_ERROR_LOG("ji_text_edit_new: out of memory"); return NULL; }
    te->text_capacity = 256;
    te->text = (char*)ji_alloc(te->text_capacity);
    if (te->text) te->text[0] = '\0';
    te->text_length = 0;
    te->selection.start = 0;
    te->selection.end = 0;
    te->cursor_pos = 0;
    te->is_read_only = false;
    te->is_rich_text = false;
    te->tab_width = 4;
    te->line_count = 1;
    te->line_wrap = true;
    return te;
}

void ji_text_edit_destroy(JiTextEdit* te) {
    if (!te) return;
    ji_free(te->text);
    ji_free(te);
}

static void ensure_capacity(JiTextEdit* te, int needed) {
    if (needed <= te->text_capacity) return;
    while (te->text_capacity < needed) te->text_capacity *= 2;
    char* new_buf = (char*)ji_alloc(te->text_capacity);
    if (!new_buf) return;
    memcpy(new_buf, te->text, te->text_length + 1);
    ji_free(te->text);
    te->text = new_buf;
}

void ji_text_edit_set_text(JiTextEdit* te, const char* text) {
    if (!te) return;
    ji_free(te->text);
    if (text) {
        size_t len = strlen(text);
        te->text_capacity = (int)len + 1;
        if (te->text_capacity < 256) te->text_capacity = 256;
        te->text = (char*)ji_alloc(te->text_capacity);
        if (te->text) { memcpy(te->text, text, len + 1); te->text_length = (int)len; }
        else { te->text_length = 0; }
    } else {
        te->text_capacity = 256;
        te->text = (char*)ji_alloc(te->text_capacity);
        if (te->text) te->text[0] = '\0';
        te->text_length = 0;
    }
    te->cursor_pos = te->text_length;
    te->selection.start = te->selection.end = 0;
}

const char* ji_text_edit_get_text(const JiTextEdit* te) { return te ? te->text : NULL; }
void ji_text_edit_set_read_only(JiTextEdit* te, bool read_only) { if (te) te->is_read_only = read_only; }
bool ji_text_edit_is_read_only(const JiTextEdit* te) { return te ? te->is_read_only : false; }
void ji_text_edit_set_cursor_position(JiTextEdit* te, int pos) { if (te && pos >= 0 && pos <= te->text_length) te->cursor_pos = pos; }
int ji_text_edit_get_cursor_position(const JiTextEdit* te) { return te ? te->cursor_pos : 0; }

void ji_text_edit_select_all(JiTextEdit* te) {
    if (!te) return;
    te->selection.start = 0;
    te->selection.end = te->text_length;
}

void ji_text_edit_set_selection(JiTextEdit* te, int start, int end) {
    if (!te) return;
    te->selection.start = start < 0 ? 0 : (start > te->text_length ? te->text_length : start);
    te->selection.end = end < 0 ? 0 : (end > te->text_length ? te->text_length : end);
}

void ji_text_edit_clear_selection(JiTextEdit* te) {
    if (!te) return;
    te->selection.start = te->selection.end = te->cursor_pos;
}

void ji_text_edit_insert_text(JiTextEdit* te, const char* text) {
    if (!te || !text) return;
    size_t len = strlen(text);
    ensure_capacity(te, te->text_length + (int)len + 1);
    memmove(te->text + te->cursor_pos + len, te->text + te->cursor_pos, te->text_length - te->cursor_pos + 1);
    memcpy(te->text + te->cursor_pos, text, len);
    te->text_length += (int)len;
    te->cursor_pos += (int)len;
}

void ji_text_edit_append_text(JiTextEdit* te, const char* text) {
    if (!te || !text) return;
    size_t len = strlen(text);
    ensure_capacity(te, te->text_length + (int)len + 1);
    memcpy(te->text + te->text_length, text, len);
    te->text_length += (int)len;
    te->text[te->text_length] = '\0';
}
