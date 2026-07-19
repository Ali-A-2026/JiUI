/**
 * JiUI - Model/View Framework implementation
 */

#include <jiui/ji_model_view.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

/* =========================================================================
 * ModelIndex
 * ========================================================================= */
JiModelIndex ji_model_index_new(int row, int column, void* internal_ptr, JiAbstractItemModel* model) {
    JiModelIndex idx = { row, column, internal_ptr, model };
    return idx;
}

bool ji_model_index_is_valid(const JiModelIndex* index) {
    return index && index->model && index->row >= 0 && index->column >= 0;
}

bool ji_model_index_equals(const JiModelIndex* a, const JiModelIndex* b) {
    if (!a || !b) return false;
    return a->row == b->row && a->column == b->column && a->internal_pointer == b->internal_pointer && a->model == b->model;
}

/* =========================================================================
 * AbstractItemModel convenience wrappers
 * ========================================================================= */
int ji_model_row_count(JiAbstractItemModel* model, const JiModelIndex* parent) {
    if (!model || !model->vtable.row_count) return 0;
    return model->vtable.row_count(model, parent);
}

int ji_model_column_count(JiAbstractItemModel* model, const JiModelIndex* parent) {
    if (!model || !model->vtable.column_count) return 0;
    return model->vtable.column_count(model, parent);
}

char* ji_model_data(JiAbstractItemModel* model, const JiModelIndex* index, JiItemDataRole role) {
    if (!model || !model->vtable.data) return NULL;
    return model->vtable.data(model, index, role);
}

bool ji_model_set_data(JiAbstractItemModel* model, const JiModelIndex* index, const char* value, JiItemDataRole role) {
    if (!model || !model->vtable.set_data) return false;
    return model->vtable.set_data(model, index, value, role);
}

JiModelIndex ji_model_index(JiAbstractItemModel* model, int row, int column, const JiModelIndex* parent) {
    if (!model || !model->vtable.index_fn) {
        JiModelIndex idx = { row, column, NULL, model };
        return idx;
    }
    return model->vtable.index_fn(model, row, column, parent);
}

JiModelIndex ji_model_parent(JiAbstractItemModel* model, const JiModelIndex* child) {
    if (!model || !model->vtable.parent_fn) {
        JiModelIndex idx = { -1, -1, NULL, NULL };
        return idx;
    }
    return model->vtable.parent_fn(model, child);
}

bool ji_model_insert_rows(JiAbstractItemModel* model, int row, int count, const JiModelIndex* parent) {
    if (!model || !model->vtable.insert_rows) return false;
    return model->vtable.insert_rows(model, row, count, parent);
}

bool ji_model_remove_rows(JiAbstractItemModel* model, int row, int count, const JiModelIndex* parent) {
    if (!model || !model->vtable.remove_rows) return false;
    return model->vtable.remove_rows(model, row, count, parent);
}

void ji_model_destroy(JiAbstractItemModel* model) {
    if (!model) return;
    if (model->vtable.destroy) model->vtable.destroy(model);
}

/* =========================================================================
 * StringListModel
 * ========================================================================= */
static int slm_row_count(JiAbstractItemModel* m, const JiModelIndex* parent) {
    (void)parent;
    JiStringListModel* slm = (JiStringListModel*)m;
    return slm->string_count;
}

static int slm_column_count(JiAbstractItemModel* m, const JiModelIndex* parent) {
    (void)m; (void)parent;
    return 1;
}

static char* slm_data(JiAbstractItemModel* m, const JiModelIndex* index, JiItemDataRole role) {
    (void)role;
    JiStringListModel* slm = (JiStringListModel*)m;
    if (!index || index->row < 0 || index->row >= slm->string_count) return NULL;
    if (role != JI_ROLE_DISPLAY) return NULL;
    return slm->strings[index->row];
}

static bool slm_set_data(JiAbstractItemModel* m, const JiModelIndex* index, const char* value, JiItemDataRole role) {
    (void)role;
    JiStringListModel* slm = (JiStringListModel*)m;
    if (!index || index->row < 0 || index->row >= slm->string_count) return false;
    ji_free(slm->strings[index->row]);
    if (value) {
        size_t len = strlen(value);
        slm->strings[index->row] = (char*)ji_alloc(len + 1);
        if (slm->strings[index->row]) memcpy(slm->strings[index->row], value, len + 1);
    } else {
        slm->strings[index->row] = NULL;
    }
    return true;
}

static bool slm_insert_rows(JiAbstractItemModel* m, int row, int count, const JiModelIndex* parent) {
    (void)parent;
    JiStringListModel* slm = (JiStringListModel*)m;
    if (row < 0 || row > slm->string_count) return false;
    while (slm->string_count + count > slm->string_capacity) {
        slm->string_capacity *= 2;
        char** new_arr = (char**)ji_alloc(sizeof(char*) * slm->string_capacity);
        if (!new_arr) return false;
        memcpy(new_arr, slm->strings, sizeof(char*) * slm->string_count);
        ji_free(slm->strings);
        slm->strings = new_arr;
    }
    memmove(slm->strings + row + count, slm->strings + row, sizeof(char*) * (slm->string_count - row));
    for (int i = 0; i < count; i++) slm->strings[row + i] = NULL;
    slm->string_count += count;
    return true;
}

static bool slm_remove_rows(JiAbstractItemModel* m, int row, int count, const JiModelIndex* parent) {
    (void)parent;
    JiStringListModel* slm = (JiStringListModel*)m;
    if (row < 0 || row + count > slm->string_count) return false;
    for (int i = 0; i < count; i++) ji_free(slm->strings[row + i]);
    memmove(slm->strings + row, slm->strings + row + count, sizeof(char*) * (slm->string_count - row - count));
    slm->string_count -= count;
    return true;
}

static void slm_destroy(JiAbstractItemModel* m) {
    JiStringListModel* slm = (JiStringListModel*)m;
    for (int i = 0; i < slm->string_count; i++) ji_free(slm->strings[i]);
    ji_free(slm->strings);
    ji_free(slm);
}

JiStringListModel* ji_string_list_model_new(void) {
    JiStringListModel* slm = (JiStringListModel*)ji_calloc(1, sizeof(JiStringListModel));
    if (!slm) { JI_ERROR_LOG("ji_string_list_model_new: out of memory"); return NULL; }
    slm->base.vtable = (JiAbstractItemModelVTable){
        .row_count = slm_row_count, .column_count = slm_column_count,
        .data = slm_data, .set_data = slm_set_data,
        .insert_rows = slm_insert_rows, .remove_rows = slm_remove_rows,
        .destroy = slm_destroy
    };
    slm->string_capacity = 16;
    slm->strings = (char**)ji_calloc(slm->string_capacity, sizeof(char*));
    return slm;
}

void ji_string_list_model_destroy(JiStringListModel* model) { ji_model_destroy((JiAbstractItemModel*)model); }

void ji_string_list_model_set_strings(JiStringListModel* model, const char** strings, int count) {
    if (!model) return;
    for (int i = 0; i < model->string_count; i++) ji_free(model->strings[i]);
    model->string_count = 0;
    if (count > model->string_capacity) {
        ji_free(model->strings);
        model->string_capacity = count + 16;
        model->strings = (char**)ji_calloc(model->string_capacity, sizeof(char*));
    }
    for (int i = 0; i < count; i++) {
        if (strings[i]) {
            size_t len = strlen(strings[i]);
            model->strings[i] = (char*)ji_alloc(len + 1);
            if (model->strings[i]) memcpy(model->strings[i], strings[i], len + 1);
        }
    }
    model->string_count = count;
}

void ji_string_list_model_append(JiStringListModel* model, const char* str) {
    if (!model) return;
    if (model->string_count >= model->string_capacity) {
        model->string_capacity *= 2;
        char** new_arr = (char**)ji_alloc(sizeof(char*) * model->string_capacity);
        if (!new_arr) return;
        memcpy(new_arr, model->strings, sizeof(char*) * model->string_count);
        ji_free(model->strings);
        model->strings = new_arr;
    }
    if (str) {
        size_t len = strlen(str);
        model->strings[model->string_count] = (char*)ji_alloc(len + 1);
        if (model->strings[model->string_count]) memcpy(model->strings[model->string_count], str, len + 1);
    } else {
        model->strings[model->string_count] = NULL;
    }
    model->string_count++;
}

void ji_string_list_model_clear(JiStringListModel* model) {
    if (!model) return;
    for (int i = 0; i < model->string_count; i++) ji_free(model->strings[i]);
    model->string_count = 0;
}

/* =========================================================================
 * AbstractItemView
 * ========================================================================= */
void ji_item_view_set_model(JiAbstractItemView* view, JiAbstractItemModel* model) {
    if (view) view->model = model;
}

void ji_item_view_set_current_index(JiAbstractItemView* view, JiModelIndex index) {
    if (view) view->current_index = index;
}

void ji_item_view_select(JiAbstractItemView* view, JiModelIndex index) {
    if (!view) return;
    if (view->selected_count >= view->selected_capacity) {
        view->selected_capacity = view->selected_capacity ? view->selected_capacity * 2 : 16;
        JiModelIndex* new_arr = (JiModelIndex*)ji_alloc(sizeof(JiModelIndex) * view->selected_capacity);
        if (!new_arr) return;
        memcpy(new_arr, view->selected_indices, sizeof(JiModelIndex) * view->selected_count);
        ji_free(view->selected_indices);
        view->selected_indices = new_arr;
    }
    view->selected_indices[view->selected_count++] = index;
}

void ji_item_view_clear_selection(JiAbstractItemView* view) {
    if (view) { view->selected_count = 0; }
}

void ji_item_view_scroll_to(JiAbstractItemView* view, JiModelIndex index) {
    if (!view) return;
    view->visible_row_start = index.row;
}

void ji_item_view_update_visible_range(JiAbstractItemView* view) {
    if (!view || !view->model) return;
    int total = ji_model_row_count(view->model, NULL);
    if (view->visible_row_start + view->visible_row_count > total)
        view->visible_row_start = total > view->visible_row_count ? total - view->visible_row_count : 0;
}

/* =========================================================================
 * ListView
 * ========================================================================= */
JiListView* ji_list_view_new(void) {
    JiListView* v = (JiListView*)ji_calloc(1, sizeof(JiListView));
    if (!v) { JI_ERROR_LOG("ji_list_view_new: out of memory"); return NULL; }
    v->base.row_height = 24;
    v->base.visible_row_count = 20;
    v->spacing = 0;
    v->icon_size = 16;
    return v;
}

void ji_list_view_destroy(JiListView* view) {
    if (view) { ji_free(view->base.selected_indices); ji_free(view); }
}

/* =========================================================================
 * TreeView
 * ========================================================================= */
JiTreeView* ji_tree_view_new(void) {
    JiTreeView* v = (JiTreeView*)ji_calloc(1, sizeof(JiTreeView));
    if (!v) { JI_ERROR_LOG("ji_tree_view_new: out of memory"); return NULL; }
    v->base.row_height = 24;
    v->base.visible_row_count = 20;
    v->indent = 20;
    v->is_header_visible = true;
    v->is_expands_on_double_click = true;
    v->expanded_capacity = 16;
    v->expanded_rows = (int*)ji_calloc(v->expanded_capacity, sizeof(int));
    return v;
}

void ji_tree_view_destroy(JiTreeView* view) {
    if (view) { ji_free(view->expanded_rows); ji_free(view->base.selected_indices); ji_free(view); }
}

void ji_tree_view_expand(JiTreeView* view, const JiModelIndex* index) {
    if (!view || !index) return;
    if (view->expanded_count >= view->expanded_capacity) {
        view->expanded_capacity *= 2;
        int* new_arr = (int*)ji_alloc(sizeof(int) * view->expanded_capacity);
        if (!new_arr) return;
        memcpy(new_arr, view->expanded_rows, sizeof(int) * view->expanded_count);
        ji_free(view->expanded_rows);
        view->expanded_rows = new_arr;
    }
    view->expanded_rows[view->expanded_count++] = index->row;
}

void ji_tree_view_collapse(JiTreeView* view, const JiModelIndex* index) {
    if (!view || !index) return;
    for (int i = 0; i < view->expanded_count; i++) {
        if (view->expanded_rows[i] == index->row) {
            for (int j = i; j < view->expanded_count - 1; j++) view->expanded_rows[j] = view->expanded_rows[j+1];
            view->expanded_count--;
            return;
        }
    }
}

bool ji_tree_view_is_expanded(const JiTreeView* view, const JiModelIndex* index) {
    if (!view || !index) return false;
    for (int i = 0; i < view->expanded_count; i++) {
        if (view->expanded_rows[i] == index->row) return true;
    }
    return false;
}

/* =========================================================================
 * TableView
 * ========================================================================= */
JiTableView* ji_table_view_new(void) {
    JiTableView* v = (JiTableView*)ji_calloc(1, sizeof(JiTableView));
    if (!v) { JI_ERROR_LOG("ji_table_view_new: out of memory"); return NULL; }
    v->base.row_height = 24;
    v->base.visible_row_count = 20;
    v->show_grid = true;
    v->is_corner_button_enabled = true;
    v->column_width = 120;
    v->row_height = 24;
    v->is_sorting_enabled = false;
    v->sort_column = -1;
    v->sort_ascending = true;
    return v;
}

void ji_table_view_destroy(JiTableView* view) {
    if (view) { ji_free(view->base.selected_indices); ji_free(view); }
}

void ji_table_view_sort_by_column(JiTableView* view, int column, bool ascending) {
    if (!view) return;
    view->sort_column = column;
    view->sort_ascending = ascending;
    view->is_sorting_enabled = true;
}
