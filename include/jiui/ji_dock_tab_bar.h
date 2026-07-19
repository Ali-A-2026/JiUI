/**
 * JiUI - Dock Tab Bar header
 */

#ifndef JIUI_DOCK_TAB_BAR_H
#define JIUI_DOCK_TAB_BAR_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiDockTabBarTab {
    char*  label;
    void*  widget;       /* JiDockWidget* */
    bool   is_closable;
    bool   is_enabled;
} JiDockTabBarTab;

typedef struct JiDockTabBar {
    JiControl            control;
    JiDockTabBarTab*     tabs;
    int                  tab_count;
    int                  tab_capacity;
    int                  current_index;
    int                  tab_height;       /* default 24 */
    bool                 tabs_closable;
    bool                 tabs_movable;
    int                  drag_index;       /* -1 when not dragging */
    int                  hover_index;
} JiDockTabBar;

JI_API JiDockTabBar* ji_dock_tab_bar_new(void);
JI_API void ji_dock_tab_bar_destroy(JiDockTabBar* bar);
JI_API int ji_dock_tab_bar_add_tab(JiDockTabBar* bar, const char* label, void* widget);
JI_API void ji_dock_tab_bar_remove_tab(JiDockTabBar* bar, int index);
JI_API int ji_dock_tab_bar_count(const JiDockTabBar* bar);
JI_API void ji_dock_tab_bar_set_current(JiDockTabBar* bar, int index);
JI_API int ji_dock_tab_bar_get_current(const JiDockTabBar* bar);
JI_API void ji_dock_tab_bar_set_tab_label(JiDockTabBar* bar, int index, const char* label);
JI_API const char* ji_dock_tab_bar_get_tab_label(const JiDockTabBar* bar, int index);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_DOCK_TAB_BAR_H */
