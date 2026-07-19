/**
 * JiUI - PlainTextEdit implementation
 */

#include <jiui/ji_plain_text_edit.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

JiPlainTextEdit* ji_plain_text_edit_new(void) {
    JiPlainTextEdit* te = (JiPlainTextEdit*)ji_calloc(1, sizeof(JiPlainTextEdit));
    if (!te) { JI_ERROR_LOG("ji_plain_text_edit_new: out of memory"); return NULL; }
    te->text_capacity = 256;
    te->text = (char*)ji_alloc(te->text_capacity);
    if (te->text) te->text[0] = '\0';
    te->text_length = 0;
    te->cursor_pos = 0;
    te->is_read_only = false;
    te->tab_width = 4;
    te->line_wrap = true;
    te->max_line_length = 0;
    return te;
}

void ji_plain_text_edit_destroy(JiPlainTextEdit* te) { if (te) { ji_free(te->text); ji_free(te); } }

static void ensure_capacity(JiPlainTextEdit* te, int needed) {
    if (needed <= te->text_capacity) return;
    while (te->text_capacity < needed) te->text_capacity *= 2;
    char* new_buf = (char*)ji_alloc(te->text_capacity);
    if (!new_buf) return;
    memcpy(new_buf, te->text, te->text_length + 1);
    ji_free(te->text);
    te->text = new_buf;
}

void ji_plain_text_edit_set_text(JiPlainTextEdit* te, const char* text) {
    if (!te) return;
    ji_free(te->text);
    if (text) {
        size_t len = strlen(text);
        te->text_capacity = (int)len + 1;
        if (te->text_capacity < 256) te->text_capacity = 256;
        te->text = (char*)ji_alloc(te->text_capacity);
        if (te->text) { memcpy(te->text, text, len + 1); te->text_length = (int)len; }
        else te->text_length = 0;
    } else {
        te->text_capacity = 256;
        te->text = (char*)ji_alloc(te->text_capacity);
        if (te->text) te->text[0] = '\0';
        te->text_length = 0;
    }
    te->cursor_pos = te->text_length;
}

const char* ji_plain_text_edit_get_text(const JiPlainTextEdit* te) { return te ? te->text : NULL; }
void ji_plain_text_edit_set_read_only(JiPlainTextEdit* te, bool read_only) { if (te) te->is_read_only = read_only; }

void ji_plain_text_edit_append_text(JiPlainTextEdit* te, const char* text) {
    if (!te || !text) return;
    size_t len = strlen(text);
    ensure_capacity(te, te->text_length + (int)len + 1);
    memcpy(te->text + te->text_length, text, len);
    te->text_length += (int)len;
    te->text[te->text_length] = '\0';
}

void ji_plain_text_edit_set_line_wrap(JiPlainTextEdit* te, bool wrap) { if (te) te->line_wrap = wrap; }
