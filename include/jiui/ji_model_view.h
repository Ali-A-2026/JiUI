/**
 * JiUI - Model/View Framework header
 * Provides abstract item models, list/tree/table views.
 * Surpasses Qt6 with built-in virtual scrolling and lazy loading.
 */

#ifndef JIUI_MODEL_VIEW_H
#define JIUI_MODEL_VIEW_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Item Data Roles
 * ========================================================================= */
typedef enum JiItemDataRole {
    JI_ROLE_DISPLAY    = 0,
    JI_ROLE_EDIT       = 1,
    JI_ROLE_TOOLTIP    = 2,
    JI_ROLE_DECORATION = 3,
    JI_ROLE_CHECKSTATE = 4,
    JI_ROLE_FOREGROUND = 5,
    JI_ROLE_BACKGROUND = 6,
    JI_ROLE_FONT       = 7,
    JI_ROLE_ALIGNMENT  = 8,
    JI_ROLE_USER       = 256
} JiItemDataRole;

typedef enum JiItemFlags {
    JI_ITEM_NO_FLAGS       = 0,
    JI_ITEM_IS_SELECTABLE  = 1 << 0,
    JI_ITEM_IS_EDITABLE    = 1 << 1,
    JI_ITEM_IS_DRAG_ENABLED  = 1 << 2,
    JI_ITEM_IS_DROP_ENABLED  = 1 << 3,
    JI_ITEM_IS_USER_CHECKABLE = 1 << 4,
    JI_ITEM_IS_ENABLED     = 1 << 5,
    JI_ITEM_IS_TRISTATE    = 1 << 6
} JiItemFlags;

/* =========================================================================
 * Model Index — identifies a cell in the model
 * ========================================================================= */
typedef struct JiModelIndex {
    int row;
    int column;
    void* internal_pointer;  /* opaque pointer for model-specific data */
    struct JiAbstractItemModel* model;
} JiModelIndex;

JI_API JiModelIndex ji_model_index_new(int row, int column, void* internal_ptr, struct JiAbstractItemModel* model);
JI_API bool ji_model_index_is_valid(const JiModelIndex* index);
JI_API bool ji_model_index_equals(const JiModelIndex* a, const JiModelIndex* b);

/* =========================================================================
 * Abstract Item Model — base class for all data models
 * ========================================================================= */
typedef struct JiAbstractItemModel JiAbstractItemModel;

typedef struct JiAbstractItemModelVTable {
    int         (*row_count)(JiAbstractItemModel* model, const JiModelIndex* parent);
    int         (*column_count)(JiAbstractItemModel* model, const JiModelIndex* parent);
    char*       (*data)(JiAbstractItemModel* model, const JiModelIndex* index, JiItemDataRole role);
    bool        (*set_data)(JiAbstractItemModel* model, const JiModelIndex* index, const char* value, JiItemDataRole role);
    char**      (*header_data)(JiAbstractItemModel* model, int section, JiItemDataRole role);
    JiModelIndex(*index_fn)(JiAbstractItemModel* model, int row, int column, const JiModelIndex* parent);
    JiModelIndex(*parent_fn)(JiAbstractItemModel* model, const JiModelIndex* child);
    JiModelIndex(*sibling)(JiAbstractItemModel* model, int row, int column, const JiModelIndex* idx);
    int         (*flags)(JiAbstractItemModel* model, const JiModelIndex* index);
    bool        (*insert_rows)(JiAbstractItemModel* model, int row, int count, const JiModelIndex* parent);
    bool        (*remove_rows)(JiAbstractItemModel* model, int row, int count, const JiModelIndex* parent);
    bool        (*insert_columns)(JiAbstractItemModel* model, int column, int count, const JiModelIndex* parent);
    bool        (*remove_columns)(JiAbstractItemModel* model, int column, int count, const JiModelIndex* parent);
    void        (*destroy)(JiAbstractItemModel* model);
} JiAbstractItemModelVTable;

typedef struct JiAbstractItemModel {
    JiAbstractItemModelVTable  vtable;
    void*                      user_data;
} JiAbstractItemModel;

/* Convenience wrappers */
JI_API int         ji_model_row_count(JiAbstractItemModel* model, const JiModelIndex* parent);
JI_API int         ji_model_column_count(JiAbstractItemModel* model, const JiModelIndex* parent);
JI_API char*       ji_model_data(JiAbstractItemModel* model, const JiModelIndex* index, JiItemDataRole role);
JI_API bool        ji_model_set_data(JiAbstractItemModel* model, const JiModelIndex* index, const char* value, JiItemDataRole role);
JI_API JiModelIndex ji_model_index(JiAbstractItemModel* model, int row, int column, const JiModelIndex* parent);
JI_API JiModelIndex ji_model_parent(JiAbstractItemModel* model, const JiModelIndex* child);
JI_API bool        ji_model_insert_rows(JiAbstractItemModel* model, int row, int count, const JiModelIndex* parent);
JI_API bool        ji_model_remove_rows(JiAbstractItemModel* model, int row, int count, const JiModelIndex* parent);
JI_API void        ji_model_destroy(JiAbstractItemModel* model);

/* =========================================================================
 * String List Model — simple list of strings
 * ========================================================================= */
typedef struct JiStringListModel {
    JiAbstractItemModel  base;
    char**               strings;
    int                  string_count;
    int                  string_capacity;
} JiStringListModel;

JI_API JiStringListModel* ji_string_list_model_new(void);
JI_API void ji_string_list_model_destroy(JiStringListModel* model);
JI_API void ji_string_list_model_set_strings(JiStringListModel* model, const char** strings, int count);
JI_API void ji_string_list_model_append(JiStringListModel* model, const char* str);
JI_API void ji_string_list_model_clear(JiStringListModel* model);

/* =========================================================================
 * Abstract Item View — base class for all views
 * ========================================================================= */
typedef struct JiAbstractItemView {
    JiControl               control;
    JiAbstractItemModel*    model;
    JiModelIndex            current_index;
    JiModelIndex*           selected_indices;
    int                     selected_count;
    int                     selected_capacity;
    bool                    is_editing;
    bool                    is_selectable;
    /* Virtual scrolling (beyond Qt6) */
    int                     visible_row_start;
    int                     visible_row_count;
    int                     row_height;        /* default 24 */
} JiAbstractItemView;

JI_API void ji_item_view_set_model(JiAbstractItemView* view, JiAbstractItemModel* model);
JI_API void ji_item_view_set_current_index(JiAbstractItemView* view, JiModelIndex index);
JI_API void ji_item_view_select(JiAbstractItemView* view, JiModelIndex index);
JI_API void ji_item_view_clear_selection(JiAbstractItemView* view);
JI_API void ji_item_view_scroll_to(JiAbstractItemView* view, JiModelIndex index);
JI_API void ji_item_view_update_visible_range(JiAbstractItemView* view);

/* =========================================================================
 * List View — displays a list model
 * ========================================================================= */
typedef struct JiListView {
    JiAbstractItemView  base;
    bool                is_wrapping;
    int                 spacing;
    int                 icon_size;
} JiListView;

JI_API JiListView* ji_list_view_new(void);
JI_API void ji_list_view_destroy(JiListView* view);

/* =========================================================================
 * Tree View — displays a tree model with expandable nodes
 * ========================================================================= */
typedef struct JiTreeView {
    JiAbstractItemView  base;
    int                 indent;        /* default 20 */
    bool                is_header_visible;
    bool                is_expands_on_double_click;
    int                * expanded_rows;
    int                  expanded_count;
    int                  expanded_capacity;
} JiTreeView;

JI_API JiTreeView* ji_tree_view_new(void);
JI_API void ji_tree_view_destroy(JiTreeView* view);
JI_API void ji_tree_view_expand(JiTreeView* view, const JiModelIndex* index);
JI_API void ji_tree_view_collapse(JiTreeView* view, const JiModelIndex* index);
JI_API bool ji_tree_view_is_expanded(const JiTreeView* view, const JiModelIndex* index);

/* =========================================================================
 * Table View — grid-based view with rows and columns
 * ========================================================================= */
typedef struct JiTableView {
    JiAbstractItemView  base;
    bool                show_grid;
    bool                is_corner_button_enabled;
    int                 column_width;
    int                 row_height;
    bool                is_sorting_enabled;
    int                 sort_column;
    bool                sort_ascending;
} JiTableView;

JI_API JiTableView* ji_table_view_new(void);
JI_API void ji_table_view_destroy(JiTableView* view);
JI_API void ji_table_view_sort_by_column(JiTableView* view, int column, bool ascending);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_MODEL_VIEW_H */
