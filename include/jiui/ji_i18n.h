/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_i18n.h
 * @brief Internationalization engine — runtime language switching, RTL/LTR,
 *        Arabic shaping, translation memory, CLDR plural rules, date/number
 *        formatting, Unicode bidi, font fallback per script.
 */

#ifndef JIUI_I18N_H
#define JIUI_I18N_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_types.h"
#include "ji_text_engine.h"   /* JiTextDirection */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * i18n Constants
 * ========================================================================= */

#define JI_I18N_MAX_LOCALES       64
#define JI_I18N_MAX_LANG_CODE     16
#define JI_I18N_MAX_COUNTRY_CODE   8
#define JI_I18N_MAX_TRANSLATIONS 4096
#define JI_I18N_MAX_KEY_LEN       128
#define JI_I18N_MAX_TEXT_LEN     1024
#define JI_I18N_MAX_PLURAL_FORMS   6

/* =========================================================================
 * i18n Data Structures
 * ========================================================================= */

/** Plural category (CLDR). */
typedef enum JiPluralCategory {
    JI_PLURAL_ZERO  = 0,
    JI_PLURAL_ONE   = 1,
    JI_PLURAL_TWO   = 2,
    JI_PLURAL_FEW   = 3,
    JI_PLURAL_MANY  = 4,
    JI_PLURAL_OTHER = 5
} JiPluralCategory;

/** A single translation entry. */
typedef struct JiTranslation {
    char key[JI_I18N_MAX_KEY_LEN];
    char text[JI_I18N_MAX_TEXT_LEN];
    char plural_text[JI_I18N_MAX_TEXT_LEN];   /* Plural form */
} JiTranslation;

/** Locale descriptor. */
typedef struct JiLocale {
    char language[JI_I18N_MAX_LANG_CODE];     /* e.g. "en", "ar", "fr" */
    char country[JI_I18N_MAX_COUNTRY_CODE];   /* e.g. "US", "SA", "FR" */
    JiTextDirection direction;
    char decimal_point[4];
    char thousands_sep[4];
    char date_format[64];
    char time_format[64];
    int  first_day_of_week;                    /* 0=Sunday, 1=Monday */
    JiTranslation* translations;              /* Dynamically allocated */
    int  translation_capacity;
    int  translation_count;
} JiLocale;

/** i18n engine state. */
typedef struct JiI18nEngine {
    JiLocale* locales[JI_I18N_MAX_LOCALES];   /* Pointers to heap-allocated locales */
    int      locale_count;
    int      current_locale;                   /* Index into locales[] */
    bool     fallback_to_key;                  /* If true, return key when no translation */
} JiI18nEngine;

/* =========================================================================
 * i18n API — Engine Lifecycle
 * ========================================================================= */

JI_API JiI18nEngine* ji_i18n_new(void);
JI_API void           ji_i18n_free(JiI18nEngine* engine);

/* =========================================================================
 * i18n API — Locale Management
 * ========================================================================= */

JI_API int  ji_i18n_register_locale(JiI18nEngine* engine, const JiLocale* locale);
JI_API int  ji_i18n_set_locale(JiI18nEngine* engine, const char* lang_code);
JI_API const char* ji_i18n_get_locale(const JiI18nEngine* engine);
JI_API JiTextDirection ji_i18n_get_direction(const JiI18nEngine* engine);
JI_API int  ji_i18n_get_locale_count(const JiI18nEngine* engine);

/* =========================================================================
 * i18n API — Translation
 * ========================================================================= */

JI_API const char* ji_i18n_translate(JiI18nEngine* engine, const char* key);
JI_API const char* ji_i18n_translate_plural(JiI18nEngine* engine,
                                              const char* key, int64_t count);
JI_API int  ji_i18n_add_translation(JiI18nEngine* engine, const char* key,
                                     const char* text);
JI_API int  ji_i18n_add_plural_translation(JiI18nEngine* engine, const char* key,
                                             const char* singular, const char* plural);

/* =========================================================================
 * i18n API — Plural Rules (CLDR)
 * ========================================================================= */

JI_API JiPluralCategory ji_i18n_plural_rule(const char* lang_code, int64_t count);

/* =========================================================================
 * i18n API — Formatting
 * ========================================================================= */

JI_API int  ji_i18n_format_number(const JiI18nEngine* engine, double value,
                                    char* out, int out_size);
JI_API int  ji_i18n_format_date(const JiI18nEngine* engine, int year, int month,
                                  int day, char* out, int out_size);
JI_API int  ji_i18n_format_time(const JiI18nEngine* engine, int hour, int minute,
                                  int second, char* out, int out_size);

/* =========================================================================
 * i18n API — Bidirectional Algorithm (UAX #9 subset)
 * ========================================================================= */

JI_API int  ji_i18n_bidi_process(const char* input, char* out, int out_size,
                                   JiTextDirection base_dir);
JI_API JiTextDirection ji_i18n_bidi_detect_direction(const char* text);

/* =========================================================================
 * i18n API — Built-in Locales
 * ========================================================================= */

JI_API void ji_i18n_locale_en(JiLocale* locale);   /* English */
JI_API void ji_i18n_locale_ar(JiLocale* locale);   /* Arabic */
JI_API void ji_i18n_locale_fr(JiLocale* locale);   /* French */
JI_API void ji_i18n_locale_de(JiLocale* locale);   /* German */
JI_API void ji_i18n_locale_es(JiLocale* locale);   /* Spanish */
JI_API void ji_i18n_locale_fa(JiLocale* locale);   /* Persian */
JI_API void ji_i18n_locale_he(JiLocale* locale);   /* Hebrew */
JI_API void ji_i18n_locale_ja(JiLocale* locale);   /* Japanese */
JI_API void ji_i18n_locale_zh(JiLocale* locale);   /* Chinese */
JI_API void ji_i18n_locale_ru(JiLocale* locale);   /* Russian */

#ifdef __cplusplus
}
#endif

#endif /* JIUI_I18N_H */
