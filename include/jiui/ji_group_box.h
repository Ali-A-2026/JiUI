/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_group_box.h
 * @brief GroupBox — titled border container with optional checkbox toggle.
 */

#ifndef JIUI_GROUP_BOX_H
#define JIUI_GROUP_BOX_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiGroupBox {
    JiControl  control;        /* must be first */

    char*      title;          /* title text displayed on the border */
    bool       is_checkable;   /* if true, shows a checkbox in the title */
    bool       is_checked;     /* checkbox state (when checkable) */
    JiControl* content;        /* child widget displayed inside the group */
    int        title_margin;   /* margin around title text (default 4px) */
    int        content_margin; /* margin around content (default 8px) */
} JiGroupBox;

/** Create a new group box with the given title. */
JI_API JiGroupBox* ji_group_box_new(const char* title);

/** Destroy a group box. */
JI_API void ji_group_box_destroy(JiGroupBox* box);

/** Set the title text. */
JI_API void ji_group_box_set_title(JiGroupBox* box, const char* title);

/** Get the title text. */
JI_API const char* ji_group_box_get_title(const JiGroupBox* box);

/** Set whether the group box is checkable. */
JI_API void ji_group_box_set_checkable(JiGroupBox* box, bool checkable);

/** Check if the group box is checkable. */
JI_API bool ji_group_box_is_checkable(const JiGroupBox* box);

/** Set the checked state (when checkable). */
JI_API void ji_group_box_set_checked(JiGroupBox* box, bool checked);

/** Check if the group box is checked. */
JI_API bool ji_group_box_is_checked(const JiGroupBox* box);

/** Set the content widget. */
JI_API void ji_group_box_set_content(JiGroupBox* box, JiControl* content);

/** Get the content widget. */
JI_API JiControl* ji_group_box_get_content(const JiGroupBox* box);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_GROUP_BOX_H */
