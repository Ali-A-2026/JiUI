/**
 * JiUI - Undo/Redo Framework implementation
 */

#include <jiui/ji_undo_stack.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

JiUndoCommand* ji_undo_command_new(const char* text) {
    JiUndoCommand* cmd = (JiUndoCommand*)ji_calloc(1, sizeof(JiUndoCommand));
    if (!cmd) { JI_ERROR_LOG("ji_undo_command_new: out of memory"); return NULL; }
    if (text) {
        size_t len = strlen(text);
        cmd->text = (char*)ji_alloc(len + 1);
        if (cmd->text) memcpy(cmd->text, text, len + 1);
    }
    cmd->id = -1;
    cmd->child_capacity = 2;
    cmd->children = (JiUndoCommand**)ji_alloc(sizeof(JiUndoCommand*) * cmd->child_capacity);
    return cmd;
}

void ji_undo_command_destroy(JiUndoCommand* cmd) {
    if (!cmd) return;
    for (int i = 0; i < cmd->child_count; i++) ji_undo_command_destroy(cmd->children[i]);
    ji_free(cmd->children);
    if (cmd->destroy_fn) cmd->destroy_fn(cmd);
    ji_free(cmd->text);
    ji_free(cmd);
}

void ji_undo_command_set_undo(JiUndoCommand* cmd, void (*fn)(JiUndoCommand*, void*)) { if (cmd) cmd->undo_fn = fn; }
void ji_undo_command_set_redo(JiUndoCommand* cmd, void (*fn)(JiUndoCommand*, void*)) { if (cmd) cmd->redo_fn = fn; }
void ji_undo_command_set_merge(JiUndoCommand* cmd, bool (*fn)(JiUndoCommand*, JiUndoCommand*)) { if (cmd) cmd->merge_fn = fn; }
void ji_undo_command_set_id(JiUndoCommand* cmd, int id) { if (cmd) cmd->id = id; }

void ji_undo_command_add_child(JiUndoCommand* parent, JiUndoCommand* child) {
    if (!parent || !child) return;
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity *= 2;
        JiUndoCommand** new_arr = (JiUndoCommand**)ji_alloc(sizeof(JiUndoCommand*) * parent->child_capacity);
        if (!new_arr) return;
        memcpy(new_arr, parent->children, sizeof(JiUndoCommand*) * parent->child_count);
        ji_free(parent->children);
        parent->children = new_arr;
    }
    parent->children[parent->child_count++] = child;
}

/* ---- UndoStack ---- */
JiUndoStack* ji_undo_stack_new(void) {
    JiUndoStack* s = (JiUndoStack*)ji_calloc(1, sizeof(JiUndoStack));
    if (!s) { JI_ERROR_LOG("ji_undo_stack_new: out of memory"); return NULL; }
    s->command_capacity = 16;
    s->commands = (JiUndoCommand**)ji_alloc(sizeof(JiUndoCommand*) * s->command_capacity);
    s->current_index = 0;
    s->clean_index = 0;
    s->undo_limit = 0;
    s->is_active = true;
    return s;
}

void ji_undo_stack_destroy(JiUndoStack* stack) {
    if (!stack) return;
    for (int i = 0; i < stack->command_count; i++) ji_undo_command_destroy(stack->commands[i]);
    ji_free(stack->commands);
    ji_free(stack);
}

void ji_undo_stack_push(JiUndoStack* stack, JiUndoCommand* cmd) {
    if (!stack || !cmd || !stack->is_active) return;
    /* Truncate any undone commands */
    for (int i = stack->current_index; i < stack->command_count; i++)
        ji_undo_command_destroy(stack->commands[i]);
    stack->command_count = stack->current_index;
    /* Try merge with previous command */
    if (stack->command_count > 0 && cmd->id >= 0) {
        JiUndoCommand* prev = stack->commands[stack->command_count - 1];
        if (prev->id == cmd->id && prev->merge_fn && prev->merge_fn(prev, cmd)) {
            ji_undo_command_destroy(cmd);
            return;
        }
    }
    /* Grow if needed */
    if (stack->command_count >= stack->command_capacity) {
        stack->command_capacity *= 2;
        JiUndoCommand** new_arr = (JiUndoCommand**)ji_alloc(sizeof(JiUndoCommand*) * stack->command_capacity);
        if (!new_arr) return;
        memcpy(new_arr, stack->commands, sizeof(JiUndoCommand*) * stack->command_count);
        ji_free(stack->commands);
        stack->commands = new_arr;
    }
    stack->commands[stack->command_count++] = cmd;
    stack->current_index = stack->command_count;
    /* Apply undo limit */
    if (stack->undo_limit > 0 && stack->command_count > stack->undo_limit) {
        int remove = stack->command_count - stack->undo_limit;
        for (int i = 0; i < remove; i++) ji_undo_command_destroy(stack->commands[i]);
        memmove(stack->commands, stack->commands + remove, sizeof(JiUndoCommand*) * (stack->command_count - remove));
        stack->command_count -= remove;
        stack->current_index -= remove;
        if (stack->clean_index >= 0) stack->clean_index -= remove;
    }
    /* Execute redo */
    if (cmd->redo_fn) cmd->redo_fn(cmd, cmd->user_data);
}

void ji_undo_stack_undo(JiUndoStack* stack) {
    if (!stack || stack->current_index <= 0) return;
    JiUndoCommand* cmd = stack->commands[--stack->current_index];
    if (cmd->undo_fn) cmd->undo_fn(cmd, cmd->user_data);
    for (int i = cmd->child_count - 1; i >= 0; i--) {
        JiUndoCommand* child = cmd->children[i];
        if (child->undo_fn) child->undo_fn(child, child->user_data);
    }
}

void ji_undo_stack_redo(JiUndoStack* stack) {
    if (!stack || stack->current_index >= stack->command_count) return;
    JiUndoCommand* cmd = stack->commands[stack->current_index++];
    for (int i = 0; i < cmd->child_count; i++) {
        JiUndoCommand* child = cmd->children[i];
        if (child->redo_fn) child->redo_fn(child, child->user_data);
    }
    if (cmd->redo_fn) cmd->redo_fn(cmd, cmd->user_data);
}

bool ji_undo_stack_can_undo(const JiUndoStack* stack) { return stack && stack->current_index > 0; }
bool ji_undo_stack_can_redo(const JiUndoStack* stack) { return stack && stack->current_index < stack->command_count; }

const char* ji_undo_stack_undo_text(const JiUndoStack* stack) {
    if (!stack || stack->current_index <= 0) return NULL;
    return stack->commands[stack->current_index - 1]->text;
}

const char* ji_undo_stack_redo_text(const JiUndoStack* stack) {
    if (!stack || stack->current_index >= stack->command_count) return NULL;
    return stack->commands[stack->current_index]->text;
}

void ji_undo_stack_clear(JiUndoStack* stack) {
    if (!stack) return;
    for (int i = 0; i < stack->command_count; i++) ji_undo_command_destroy(stack->commands[i]);
    stack->command_count = 0;
    stack->current_index = 0;
    stack->clean_index = 0;
}

void ji_undo_stack_set_undo_limit(JiUndoStack* stack, int limit) { if (stack) stack->undo_limit = limit; }
void ji_undo_stack_set_active(JiUndoStack* stack, bool active) { if (stack) stack->is_active = active; }
bool ji_undo_stack_is_clean(const JiUndoStack* stack) { return stack ? stack->current_index == stack->clean_index : true; }
void ji_undo_stack_set_clean(JiUndoStack* stack) { if (stack) stack->clean_index = stack->current_index; }
