/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_property.h
 * @brief Property system — styled properties, direct properties, attached
 *        properties, metadata, change notifications.
 *
 * This is the C property system supporting styled and direct properties.
 * DirectProperty system. Properties are identified by a unique JiPropertyId
 * and carry metadata (default value, coercion, validation, change callbacks).
 */

#ifndef JIUI_PROPERTY_H
#define JIUI_PROPERTY_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_type.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Forward declarations
 * ========================================================================= */
typedef struct JiObject         JiObject;
typedef struct JiProperty      JiProperty;
typedef struct JiPropertyMetadata JiPropertyMetadata;

/* =========================================================================
 * Property ID — unique integer identifying a registered property
 * ========================================================================= */
typedef int JiPropertyId;

/** Invalid/uninitialized property ID. */
#define JI_PROPERTY_INVALID ((JiPropertyId)-1)

/* =========================================================================
 * Property value type tag
 * ========================================================================= */
typedef enum JiPropertyType {
    JI_PROP_TYPE_BOOL     = 0,
    JI_PROP_TYPE_INT      = 1,
    JI_PROP_TYPE_FLOAT    = 2,
    JI_PROP_TYPE_DOUBLE   = 3,
    JI_PROP_TYPE_STRING   = 4,   /* null-terminated char* */
    JI_PROP_TYPE_PTR      = 5,   /* arbitrary pointer */
    JI_PROP_TYPE_OBJECT   = 6,   /* JiObject* */
    JI_PROP_TYPE_COLOR    = 7,   /* uint32_t ARGB */
    JI_PROP_TYPE_POINT    = 8,   /* JiPoint */
    JI_PROP_TYPE_SIZE     = 9,   /* JiSize */
    JI_PROP_TYPE_RECT     = 10,  /* JiRect */
    JI_PROP_TYPE_THICKNESS= 11,  /* JiThickness */
    JI_PROP_TYPE_VECTOR   = 12,  /* JiVector */
    JI_PROP_TYPE_MATRIX   = 13,  /* JiMatrix */
    JI_PROP_TYPE_ENUM     = 14,  /* int */
    JI_PROP_TYPE_CUSTOM   = 15   /* user-defined, stored as ptr */
} JiPropertyType;

/* =========================================================================
 * Property value union
 * ========================================================================= */
typedef struct JiPropertyValue {
    JiPropertyType type;
    union {
        bool        bool_val;
        int         int_val;
        float       float_val;
        double      double_val;
        char*       string_val;   /* owned copy when set */
        void*       ptr_val;
        JiObject*   object_val;
        uint32_t    color_val;    /* ARGB */
        /* Geometric types stored by value via pointer to copy */
        void*       custom_val;
    } v;
} JiPropertyValue;

/* =========================================================================
 * Property kind: Styled vs Direct vs Attached
 * ========================================================================= */
typedef enum JiPropertyKind {
    JI_PROP_STYLED   = 0,  /* Like Avalonia StyledProperty — supports styling, inheritance */
    JI_PROP_DIRECT   = 1,  /* Like Avalonia DirectProperty — backed by field, no styling */
    JI_PROP_ATTACHED = 2   /* Styled property registered on a different owner type */
} JiPropertyKind;

/* =========================================================================
 * Callback types
 * ========================================================================= */
typedef struct JiObject JiObject;

/** Called when a property value changes. */
typedef void (*JiPropertyChangedCallback)(JiObject* obj,
                                          JiPropertyId prop_id,
                                          const JiPropertyValue* old_val,
                                          const JiPropertyValue* new_val);

/** Coercion: validate/transform a value before it is set. Returns true if accepted. */
typedef bool (*JiCoerceCallback)(JiObject* obj,
                                  JiPropertyId prop_id,
                                  JiPropertyValue* inout_val);

/** Validation: return true if the value is acceptable. */
typedef bool (*JiValidateCallback)(JiObject* obj,
                                    JiPropertyId prop_id,
                                    const JiPropertyValue* val);

/* =========================================================================
 * JiPropertyMetadata — metadata for a property registration
 * ========================================================================= */
struct JiPropertyMetadata {
    JiPropertyValue           default_value;     /* default value for this property */
    JiPropertyChangedCallback on_changed;       /* called when value changes */
    JiCoerceCallback          coerce;            /* called to coerce/validate before set */
    JiValidateCallback        validate;          /* called to validate a value */
    bool                      inherits;          /* true if property value inherits from parent */
    bool                      binds_two_way;     /* true if default binding mode is two-way */
};

/* =========================================================================
 * JiProperty — descriptor for a registered property
 * ========================================================================= */
struct JiProperty {
    JiPropertyId       id;           /* unique property id               */
    const char*        name;         /* property name                    */
    JiTypeId           owner_type;   /* type that declares this property */
    JiPropertyKind     kind;         /* styled / direct / attached       */
    JiPropertyType     value_type;   /* type of the value                */
    JiPropertyMetadata metadata;     /* default metadata                 */
};

/* =========================================================================
 * Property registration API
 * ========================================================================= */

/**
 * Register a styled property.
 * Styled properties support styling, inheritance, and default values.
 */
JI_API JiPropertyId ji_property_register_styled(
    const char* name,
    JiTypeId owner_type,
    JiPropertyType value_type,
    JiPropertyMetadata metadata);

/**
 * Register a direct property.
 * Direct properties are backed by a field and do not support styling.
 */
JI_API JiPropertyId ji_property_register_direct(
    const char* name,
    JiTypeId owner_type,
    JiPropertyType value_type,
    JiPropertyMetadata metadata);

/**
 * Register an attached property.
 * Attached properties are styled properties owned by a different type.
 */
JI_API JiPropertyId ji_property_register_attached(
    const char* name,
    JiTypeId owner_type,
    JiPropertyType value_type,
    JiPropertyMetadata metadata);

/**
 * Look up a property by name and owner type.
 */
JI_API JiPropertyId ji_property_from_name(const char* name, JiTypeId owner_type);

/**
 * Get the property descriptor.
 */
JI_API const JiProperty* ji_property_get(JiPropertyId id);

/**
 * Get the property name.
 */
JI_API const char* ji_property_name(JiPropertyId id);

/**
 * Get the property's value type.
 */
JI_API JiPropertyType ji_property_value_type(JiPropertyId id);

/**
 * Get the property's kind.
 */
JI_API JiPropertyKind ji_property_kind(JiPropertyId id);

/**
 * Check if a property is registered on a given type or its ancestors.
 */
JI_API bool ji_property_is_registered_on(JiPropertyId prop_id, JiTypeId type_id);

/* =========================================================================
 * JiPropertyValue helpers
 * ========================================================================= */

JI_API JiPropertyValue ji_value_bool(bool v);
JI_API JiPropertyValue ji_value_int(int v);
JI_API JiPropertyValue ji_value_float(float v);
JI_API JiPropertyValue ji_value_double(double v);
JI_API JiPropertyValue ji_value_string(const char* v);   /* copies the string */
JI_API JiPropertyValue ji_value_ptr(void* v);
JI_API JiPropertyValue ji_value_object(JiObject* v);
JI_API JiPropertyValue ji_value_color(uint32_t argb);
JI_API JiPropertyValue ji_value_int_enum(int v);
JI_API JiPropertyValue ji_value_null(JiPropertyType type);

/** Free any owned memory in a JiPropertyValue (e.g. string). */
JI_API void ji_value_destroy(JiPropertyValue* val);

/** Deep-copy a value (strings are duplicated). */
JI_API JiPropertyValue ji_value_copy(const JiPropertyValue* val);

/** Compare two values for equality. */
JI_API bool ji_value_equals(const JiPropertyValue* a, const JiPropertyValue* b);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_PROPERTY_H */
