/**
 * JiUI - CommandLinkButton implementation
 */

#include <jiui/ji_command_link_button.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

JiCommandLinkButton* ji_command_link_button_new(const char* text, const char* description) {
    JiCommandLinkButton* btn = (JiCommandLinkButton*)ji_calloc(1, sizeof(JiCommandLinkButton));
    if (!btn) { JI_ERROR_LOG("ji_command_link_button_new: out of memory"); return NULL; }
    if (text) {
        size_t len = strlen(text);
        btn->text = (char*)ji_alloc(len + 1);
        if (btn->text) memcpy(btn->text, text, len + 1);
    }
    if (description) {
        size_t len = strlen(description);
        btn->description = (char*)ji_alloc(len + 1);
        if (btn->description) memcpy(btn->description, description, len + 1);
    }
    btn->is_default = false;
    return btn;
}

void ji_command_link_button_destroy(JiCommandLinkButton* btn) { if (btn) { ji_free(btn->text); ji_free(btn->description); ji_free(btn); } }

void ji_command_link_button_set_text(JiCommandLinkButton* btn, const char* text) {
    if (!btn) return;
    ji_free(btn->text);
    if (text) { size_t len = strlen(text); btn->text = (char*)ji_alloc(len+1); if (btn->text) memcpy(btn->text, text, len+1); }
    else btn->text = NULL;
}

const char* ji_command_link_button_get_text(const JiCommandLinkButton* btn) { return btn ? btn->text : NULL; }

void ji_command_link_button_set_description(JiCommandLinkButton* btn, const char* desc) {
    if (!btn) return;
    ji_free(btn->description);
    if (desc) { size_t len = strlen(desc); btn->description = (char*)ji_alloc(len+1); if (btn->description) memcpy(btn->description, desc, len+1); }
    else btn->description = NULL;
}

const char* ji_command_link_button_get_description(const JiCommandLinkButton* btn) { return btn ? btn->description : NULL; }
void ji_command_link_button_set_default(JiCommandLinkButton* btn, bool is_default) { if (btn) btn->is_default = is_default; }
bool ji_command_link_button_is_default(const JiCommandLinkButton* btn) { return btn ? btn->is_default : false; }
