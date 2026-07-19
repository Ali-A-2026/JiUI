/**
 * JiUI - Dock Serializer implementation
 */

#include <jiui/ji_dock_serializer.h>
#include <jiui/ji_dock_manager.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <stdio.h>
#include <string.h>

static void ser_ensure(JiDockSerializer* ser, int needed) {
    if (ser->buffer_size + needed <= ser->buffer_capacity) return;
    while (ser->buffer_size + needed > ser->buffer_capacity) ser->buffer_capacity *= 2;
    char* new_buf = (char*)ji_alloc(ser->buffer_capacity);
    if (!new_buf) return;
    memcpy(new_buf, ser->buffer, ser->buffer_size);
    ji_free(ser->buffer);
    ser->buffer = new_buf;
}

static void ser_append(JiDockSerializer* ser, const char* text) {
    if (!text) return;
    int len = (int)strlen(text);
    ser_ensure(ser, len + 1);
    memcpy(ser->buffer + ser->buffer_size, text, len);
    ser->buffer_size += len;
    ser->buffer[ser->buffer_size] = '\0';
}

JiDockSerializer* ji_dock_serializer_new(void) {
    JiDockSerializer* ser = (JiDockSerializer*)ji_calloc(1, sizeof(JiDockSerializer));
    if (!ser) { JI_ERROR_LOG("ji_dock_serializer_new: out of memory"); return NULL; }
    ser->buffer_capacity = 1024;
    ser->buffer = (char*)ji_alloc(ser->buffer_capacity);
    ser->buffer[0] = '\0';
    ser->buffer_size = 0;
    ser->indent_level = 0;
    return ser;
}

void ji_dock_serializer_destroy(JiDockSerializer* ser) { if (ser) { ji_free(ser->buffer); ji_free(ser); } }

void ji_dock_serializer_save(JiDockSerializer* ser, JiDockManager* manager) {
    if (!ser || !manager) return;
    ser->buffer_size = 0;
    ser->buffer[0] = '\0';

    char tmp[256];
    snprintf(tmp, sizeof(tmp), "dock_manager\n");
    ser_append(ser, tmp);
    ser_append(ser, "  area_count=");
    snprintf(tmp, sizeof(tmp), "%d\n", manager->area_count);
    ser_append(ser, tmp);
    ser_append(ser, "  floating_count=");
    snprintf(tmp, sizeof(tmp), "%d\n", manager->floating_count);
    ser_append(ser, tmp);

    for (int i = 0; i < manager->floating_count; i++) {
        JiDockWidget* w = manager->floating_widgets[i];
        if (!w) continue;
        ser_append(ser, "  dock_widget\n");
        ser_append(ser, "    title=");
        if (w->title) ser_append(ser, w->title);
        ser_append(ser, "\n");
        snprintf(tmp, sizeof(tmp), "    state=%d\n", (int)w->state);
        ser_append(ser, tmp);
        snprintf(tmp, sizeof(tmp), "    features=%d\n", w->features);
        ser_append(ser, tmp);
    }
}

const char* ji_dock_serializer_get_data(const JiDockSerializer* ser) { return ser ? ser->buffer : NULL; }

bool ji_dock_serializer_load(JiDockSerializer* ser, JiDockManager* manager, const char* data) {
    if (!ser || !manager || !data) return false;
    /* Simple validation: check header */
    if (strncmp(data, "dock_manager", 12) != 0) return false;
    /* Full parsing would require a proper parser; for now, just validate header */
    return true;
}
