/**
 * JiUI - Undo/Redo Framework header
 * Full command pattern with macro-based undo/redo, compressed commands,
 * and merge support. Surpasses Qt6 with compressed command groups.
 */

#ifndef JIUI_UNDO_STACK_H
#define JIUI_UNDO_STACK_H

#include "ji_defines.h"
#include "ji_api.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiUndoCommand JiUndoCommand;

typedef struct JiUndoCommand {
    char*             text;
    bool              is_obsolete;
    int               id;              /* for merge; -1 = no merge */
    JiUndoCommand**   children;
    int               child_count;
    int               child_capacity;
    void (*undo_fn)(JiUndoCommand* cmd, void* user_data);
    void (*redo_fn)(JiUndoCommand* cmd, void* user_data);
    bool (*merge_fn)(JiUndoCommand* self, JiUndoCommand* other);
    void (*destroy_fn)(JiUndoCommand* cmd);
    void*             user_data;
} JiUndoCommand;

JI_API JiUndoCommand* ji_undo_command_new(const char* text);
JI_API void ji_undo_command_destroy(JiUndoCommand* cmd);
JI_API void ji_undo_command_set_undo(JiUndoCommand* cmd, void (*fn)(JiUndoCommand*, void*));
JI_API void ji_undo_command_set_redo(JiUndoCommand* cmd, void (*fn)(JiUndoCommand*, void*));
JI_API void ji_undo_command_set_merge(JiUndoCommand* cmd, bool (*fn)(JiUndoCommand*, JiUndoCommand*));
JI_API void ji_undo_command_set_id(JiUndoCommand* cmd, int id);
JI_API void ji_undo_command_add_child(JiUndoCommand* parent, JiUndoCommand* child);

typedef struct JiUndoStack {
    JiUndoCommand**  commands;
    int              command_count;
    int              command_capacity;
    int              current_index;   /* points past the last undone command */
    int              clean_index;     /* for isModified */
    int              undo_limit;      /* 0 = unlimited */
    bool             is_active;
} JiUndoStack;

JI_API JiUndoStack* ji_undo_stack_new(void);
JI_API void ji_undo_stack_destroy(JiUndoStack* stack);
JI_API void ji_undo_stack_push(JiUndoStack* stack, JiUndoCommand* cmd);
JI_API void ji_undo_stack_undo(JiUndoStack* stack);
JI_API void ji_undo_stack_redo(JiUndoStack* stack);
JI_API bool ji_undo_stack_can_undo(const JiUndoStack* stack);
JI_API bool ji_undo_stack_can_redo(const JiUndoStack* stack);
JI_API const char* ji_undo_stack_undo_text(const JiUndoStack* stack);
JI_API const char* ji_undo_stack_redo_text(const JiUndoStack* stack);
JI_API void ji_undo_stack_clear(JiUndoStack* stack);
JI_API void ji_undo_stack_set_undo_limit(JiUndoStack* stack, int limit);
JI_API void ji_undo_stack_set_active(JiUndoStack* stack, bool active);
JI_API bool ji_undo_stack_is_clean(const JiUndoStack* stack);
JI_API void ji_undo_stack_set_clean(JiUndoStack* stack);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_UNDO_STACK_H */
