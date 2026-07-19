/**
 * JiUI - CommandLinkButton widget header
 */

#ifndef JIUI_COMMAND_LINK_BUTTON_H
#define JIUI_COMMAND_LINK_BUTTON_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiCommandLinkButton {
    JiControl  control;
    char*      text;
    char*      description;  /* secondary descriptive text below the main text */
    bool       is_default;   /* auto-default button */
} JiCommandLinkButton;

JI_API JiCommandLinkButton* ji_command_link_button_new(const char* text, const char* description);
JI_API void ji_command_link_button_destroy(JiCommandLinkButton* btn);
JI_API void ji_command_link_button_set_text(JiCommandLinkButton* btn, const char* text);
JI_API const char* ji_command_link_button_get_text(const JiCommandLinkButton* btn);
JI_API void ji_command_link_button_set_description(JiCommandLinkButton* btn, const char* desc);
JI_API const char* ji_command_link_button_get_description(const JiCommandLinkButton* btn);
JI_API void ji_command_link_button_set_default(JiCommandLinkButton* btn, bool is_default);
JI_API bool ji_command_link_button_is_default(const JiCommandLinkButton* btn);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_COMMAND_LINK_BUTTON_H */
