/**
 * JiUI - GroupBox implementation
 */

#include <jiui/ji_group_box.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

JiGroupBox* ji_group_box_new(const char* title) {
    JiGroupBox* box = (JiGroupBox*)ji_calloc(1, sizeof(JiGroupBox));
    if (!box) { JI_ERROR_LOG("ji_group_box_new: out of memory"); return NULL; }
    if (title) {
        size_t len = strlen(title);
        box->title = (char*)ji_alloc(len + 1);
        if (box->title) memcpy(box->title, title, len + 1);
    }
    box->is_checkable = false;
    box->is_checked = true;
    box->title_margin = 4;
    box->content_margin = 8;
    return box;
}

void ji_group_box_destroy(JiGroupBox* box) {
    if (!box) return;
    ji_free(box->title);
    ji_free(box);
}

void ji_group_box_set_title(JiGroupBox* box, const char* title) {
    if (!box) return;
    ji_free(box->title);
    if (title) {
        size_t len = strlen(title);
        box->title = (char*)ji_alloc(len + 1);
        if (box->title) memcpy(box->title, title, len + 1);
    } else { box->title = NULL; }
}

const char* ji_group_box_get_title(const JiGroupBox* box) { return box ? box->title : NULL; }
void ji_group_box_set_checkable(JiGroupBox* box, bool checkable) { if (box) box->is_checkable = checkable; }
bool ji_group_box_is_checkable(const JiGroupBox* box) { return box ? box->is_checkable : false; }
void ji_group_box_set_checked(JiGroupBox* box, bool checked) { if (box) box->is_checked = checked; }
bool ji_group_box_is_checked(const JiGroupBox* box) { return box ? box->is_checked : false; }
void ji_group_box_set_content(JiGroupBox* box, JiControl* content) { if (box) box->content = content; }
JiControl* ji_group_box_get_content(const JiGroupBox* box) { return box ? box->content : NULL; }
