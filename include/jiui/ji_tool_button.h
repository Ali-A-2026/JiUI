/**
 * JiUI - ToolButton widget header
 */

#ifndef JIUI_TOOL_BUTTON_H
#define JIUI_TOOL_BUTTON_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JiToolButtonStyle {
    JI_TOOL_BUTTON_ICON_ONLY = 0,
    JI_TOOL_BUTTON_TEXT_ONLY = 1,
    JI_TOOL_BUTTON_TEXT_BESIDE_ICON = 2,
    JI_TOOL_BUTTON_TEXT_UNDER_ICON = 3
} JiToolButtonStyle;

typedef enum JiToolButtonPopupMode {
    JI_TOOL_BUTTON_DELAYED_POPUP = 0,
    JI_TOOL_BUTTON_MENU_BUTTON_POPUP = 1,
    JI_TOOL_BUTTON_INSTANT_POPUP = 2
} JiToolButtonPopupMode;

typedef struct JiMenu JiMenu;

typedef struct JiToolButton {
    JiControl              control;
    char*                  text;
    char*                  icon_name;
    JiToolButtonStyle      button_style;
    JiToolButtonPopupMode  popup_mode;
    struct JiMenu*         menu;
    bool                   is_checked;
    bool                   is_checkable;
    bool                   is_auto_repeat;
    int                    icon_size;
    bool                   is_arrow_visible;
} JiToolButton;

JI_API JiToolButton* ji_tool_button_new(const char* text);
JI_API void ji_tool_button_destroy(JiToolButton* btn);
JI_API void ji_tool_button_set_text(JiToolButton* btn, const char* text);
JI_API const char* ji_tool_button_get_text(const JiToolButton* btn);
JI_API void ji_tool_button_set_icon(JiToolButton* btn, const char* icon_name);
JI_API void ji_tool_button_set_style(JiToolButton* btn, JiToolButtonStyle style);
JI_API void ji_tool_button_set_checkable(JiToolButton* btn, bool checkable);
JI_API void ji_tool_button_set_checked(JiToolButton* btn, bool checked);
JI_API bool ji_tool_button_is_checked(const JiToolButton* btn);
JI_API void ji_tool_button_set_menu(JiToolButton* btn, struct JiMenu* menu);
JI_API void ji_tool_button_set_popup_mode(JiToolButton* btn, JiToolButtonPopupMode mode);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_TOOL_BUTTON_H */
