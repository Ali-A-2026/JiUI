/**
 * JiUI - ToolButton implementation
 */

#include <jiui/ji_tool_button.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

JiToolButton* ji_tool_button_new(const char* text) {
    JiToolButton* btn = (JiToolButton*)ji_calloc(1, sizeof(JiToolButton));
    if (!btn) { JI_ERROR_LOG("ji_tool_button_new: out of memory"); return NULL; }
    if (text) {
        size_t len = strlen(text);
        btn->text = (char*)ji_alloc(len + 1);
        if (btn->text) memcpy(btn->text, text, len + 1);
    }
    btn->button_style = JI_TOOL_BUTTON_TEXT_BESIDE_ICON;
    btn->popup_mode = JI_TOOL_BUTTON_DELAYED_POPUP;
    btn->menu = NULL;
    btn->is_checked = false;
    btn->is_checkable = false;
    btn->is_auto_repeat = false;
    btn->icon_size = 24;
    btn->is_arrow_visible = false;
    return btn;
}

void ji_tool_button_destroy(JiToolButton* btn) { if (btn) { ji_free(btn->text); ji_free(btn->icon_name); ji_free(btn); } }

void ji_tool_button_set_text(JiToolButton* btn, const char* text) {
    if (!btn) return;
    ji_free(btn->text);
    if (text) { size_t len = strlen(text); btn->text = (char*)ji_alloc(len+1); if (btn->text) memcpy(btn->text, text, len+1); }
    else btn->text = NULL;
}

const char* ji_tool_button_get_text(const JiToolButton* btn) { return btn ? btn->text : NULL; }

void ji_tool_button_set_icon(JiToolButton* btn, const char* icon_name) {
    if (!btn) return;
    ji_free(btn->icon_name);
    if (icon_name) { size_t len = strlen(icon_name); btn->icon_name = (char*)ji_alloc(len+1); if (btn->icon_name) memcpy(btn->icon_name, icon_name, len+1); }
    else btn->icon_name = NULL;
}

void ji_tool_button_set_style(JiToolButton* btn, JiToolButtonStyle style) { if (btn) btn->button_style = style; }
void ji_tool_button_set_checkable(JiToolButton* btn, bool checkable) { if (btn) { btn->is_checkable = checkable; if (!checkable) btn->is_checked = false; } }
void ji_tool_button_set_checked(JiToolButton* btn, bool checked) { if (btn && btn->is_checkable) btn->is_checked = checked; }
bool ji_tool_button_is_checked(const JiToolButton* btn) { return btn ? btn->is_checked : false; }
void ji_tool_button_set_menu(JiToolButton* btn, struct JiMenu* menu) { if (btn) { btn->menu = menu; btn->is_arrow_visible = (menu != NULL); } }
void ji_tool_button_set_popup_mode(JiToolButton* btn, JiToolButtonPopupMode mode) { if (btn) btn->popup_mode = mode; }
