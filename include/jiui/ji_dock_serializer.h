/**
 * JiUI - Dock Serializer header
 */

#ifndef JIUI_DOCK_SERIALIZER_H
#define JIUI_DOCK_SERIALIZER_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiDockManager JiDockManager;

/* Serialization format: simple key-value text format */
typedef struct JiDockSerializer {
    char*    buffer;
    int      buffer_size;
    int      buffer_capacity;
    int      indent_level;
} JiDockSerializer;

JI_API JiDockSerializer* ji_dock_serializer_new(void);
JI_API void ji_dock_serializer_destroy(JiDockSerializer* ser);

/* Serialize the entire dock manager state to the internal buffer */
JI_API void ji_dock_serializer_save(JiDockSerializer* ser, JiDockManager* manager);

/* Get the serialized state as a string (valid until next save or destroy) */
JI_API const char* ji_dock_serializer_get_data(const JiDockSerializer* ser);

/* Restore dock manager state from a string */
JI_API bool ji_dock_serializer_load(JiDockSerializer* ser, JiDockManager* manager, const char* data);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_DOCK_SERIALIZER_H */
