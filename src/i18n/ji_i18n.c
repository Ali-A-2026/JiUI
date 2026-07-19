/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_i18n.c
 * @brief Internationalization engine implementation.
 */

#include "jiui/ji_i18n.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* =========================================================================
 * Engine Lifecycle
 * ========================================================================= */

JiI18nEngine* ji_i18n_new(void)
{
    JiI18nEngine* engine = (JiI18nEngine*)calloc(1, sizeof(JiI18nEngine));
    if (!engine) return NULL;
    engine->current_locale = -1;
    engine->fallback_to_key = true;
    return engine;
}

void ji_i18n_free(JiI18nEngine* engine)
{
    if (engine) {
        for (int i = 0; i < engine->locale_count; i++) {
            if (engine->locales[i]) {
                free(engine->locales[i]->translations);
                free(engine->locales[i]);
            }
        }
        free(engine);
    }
}

/* =========================================================================
 * Locale Management
 * ========================================================================= */

static int find_locale_by_lang(const JiI18nEngine* engine, const char* lang)
{
    for (int i = 0; i < engine->locale_count; i++) {
        if (strcmp(engine->locales[i]->language, lang) == 0)
            return i;
    }
    return -1;
}

int ji_i18n_register_locale(JiI18nEngine* engine, const JiLocale* locale)
{
    if (!engine || !locale) return -1;
    if (engine->locale_count >= JI_I18N_MAX_LOCALES) return -1;
    /* Check if locale already exists */
    int idx = find_locale_by_lang(engine, locale->language);
    if (idx >= 0) {
        /* Replace existing: free old translations, deep-copy new */
        free(engine->locales[idx]->translations);
        *engine->locales[idx] = *locale;
        /* Deep-copy translations array */
        if (locale->translation_count > 0 && locale->translations) {
            engine->locales[idx]->translations =
                (JiTranslation*)malloc(locale->translation_count * sizeof(JiTranslation));
            if (engine->locales[idx]->translations) {
                memcpy(engine->locales[idx]->translations, locale->translations,
                       locale->translation_count * sizeof(JiTranslation));
            }
        } else {
            engine->locales[idx]->translations = NULL;
        }
        return idx;
    }
    /* Allocate new locale on heap and deep-copy */
    JiLocale* copy = (JiLocale*)malloc(sizeof(JiLocale));
    if (!copy) return -1;
    *copy = *locale;
    if (locale->translation_count > 0 && locale->translations) {
        copy->translations = (JiTranslation*)malloc(locale->translation_count * sizeof(JiTranslation));
        if (!copy->translations) {
            free(copy);
            return -1;
        }
        memcpy(copy->translations, locale->translations,
               locale->translation_count * sizeof(JiTranslation));
    } else {
        copy->translations = NULL;
    }
    engine->locales[engine->locale_count] = copy;
    return engine->locale_count++;
}

int ji_i18n_set_locale(JiI18nEngine* engine, const char* lang_code)
{
    if (!engine || !lang_code) return -1;
    int idx = find_locale_by_lang(engine, lang_code);
    if (idx < 0) return -1;
    engine->current_locale = idx;
    return 0;
}

const char* ji_i18n_get_locale(const JiI18nEngine* engine)
{
    if (!engine || engine->current_locale < 0) return "en";
    return engine->locales[engine->current_locale]->language;
}

JiTextDirection ji_i18n_get_direction(const JiI18nEngine* engine)
{
    if (!engine || engine->current_locale < 0) return JI_TEXT_DIR_LTR;
    return engine->locales[engine->current_locale]->direction;
}

int ji_i18n_get_locale_count(const JiI18nEngine* engine)
{
    return engine ? engine->locale_count : 0;
}

/* =========================================================================
 * Translation
 * ========================================================================= */

static const JiTranslation* find_translation(const JiLocale* locale, const char* key)
{
    for (int i = 0; i < locale->translation_count; i++) {
        if (strcmp(locale->translations[i].key, key) == 0)
            return &locale->translations[i];
    }
    return NULL;
}

const char* ji_i18n_translate(JiI18nEngine* engine, const char* key)
{
    if (!engine || !key) return key ? key : "";
    if (engine->current_locale < 0) return key;
    const JiLocale* loc = engine->locales[engine->current_locale];
    const JiTranslation* t = find_translation(loc, key);
    if (t) return t->text;
    /* Fallback: search other locales */
    for (int i = 0; i < engine->locale_count; i++) {
        if (i == engine->current_locale) continue;
        t = find_translation(engine->locales[i], key);
        if (t) return t->text;
    }
    return engine->fallback_to_key ? key : "";
}

const char* ji_i18n_translate_plural(JiI18nEngine* engine, const char* key, int64_t count)
{
    if (!engine || !key) return key ? key : "";
    if (engine->current_locale < 0) return key;
    const JiLocale* loc = engine->locales[engine->current_locale];
    const JiTranslation* t = find_translation(loc, key);
    if (!t) return engine->fallback_to_key ? key : "";

    JiPluralCategory cat = ji_i18n_plural_rule(loc->language, count);
    if (cat == JI_PLURAL_ONE) return t->text;
    return t->plural_text[0] ? t->plural_text : t->text;
}

int ji_i18n_add_translation(JiI18nEngine* engine, const char* key, const char* text)
{
    if (!engine || !key || !text) return -1;
    if (engine->current_locale < 0) return -1;
    JiLocale* loc = engine->locales[engine->current_locale];
    if (loc->translation_count >= JI_I18N_MAX_TRANSLATIONS) return -1;
    /* Allocate translations array if not yet allocated */
    if (!loc->translations) {
        loc->translations = (JiTranslation*)calloc(JI_I18N_MAX_TRANSLATIONS, sizeof(JiTranslation));
        if (!loc->translations) return -1;
        loc->translation_capacity = JI_I18N_MAX_TRANSLATIONS;
    }
    /* Check if key exists */
    for (int i = 0; i < loc->translation_count; i++) {
        if (strcmp(loc->translations[i].key, key) == 0) {
            strncpy(loc->translations[i].text, text, JI_I18N_MAX_TEXT_LEN - 1);
            loc->translations[i].text[JI_I18N_MAX_TEXT_LEN - 1] = '\0';
            return 0;
        }
    }
    JiTranslation* t = &loc->translations[loc->translation_count++];
    strncpy(t->key, key, JI_I18N_MAX_KEY_LEN - 1);
    t->key[JI_I18N_MAX_KEY_LEN - 1] = '\0';
    strncpy(t->text, text, JI_I18N_MAX_TEXT_LEN - 1);
    t->text[JI_I18N_MAX_TEXT_LEN - 1] = '\0';
    t->plural_text[0] = '\0';
    return 0;
}

int ji_i18n_add_plural_translation(JiI18nEngine* engine, const char* key,
                                     const char* singular, const char* plural)
{
    if (!engine || !key || !singular || !plural) return -1;
    if (engine->current_locale < 0) return -1;
    JiLocale* loc = engine->locales[engine->current_locale];
    if (loc->translation_count >= JI_I18N_MAX_TRANSLATIONS) return -1;
    /* Allocate translations array if not yet allocated */
    if (!loc->translations) {
        loc->translations = (JiTranslation*)calloc(JI_I18N_MAX_TRANSLATIONS, sizeof(JiTranslation));
        if (!loc->translations) return -1;
        loc->translation_capacity = JI_I18N_MAX_TRANSLATIONS;
    }
    for (int i = 0; i < loc->translation_count; i++) {
        if (strcmp(loc->translations[i].key, key) == 0) {
            strncpy(loc->translations[i].text, singular, JI_I18N_MAX_TEXT_LEN - 1);
            strncpy(loc->translations[i].plural_text, plural, JI_I18N_MAX_TEXT_LEN - 1);
            return 0;
        }
    }
    JiTranslation* t = &loc->translations[loc->translation_count++];
    strncpy(t->key, key, JI_I18N_MAX_KEY_LEN - 1);
    t->key[JI_I18N_MAX_KEY_LEN - 1] = '\0';
    strncpy(t->text, singular, JI_I18N_MAX_TEXT_LEN - 1);
    t->text[JI_I18N_MAX_TEXT_LEN - 1] = '\0';
    strncpy(t->plural_text, plural, JI_I18N_MAX_TEXT_LEN - 1);
    t->plural_text[JI_I18N_MAX_TEXT_LEN - 1] = '\0';
    return 0;
}

/* =========================================================================
 * Formatting
 * ========================================================================= */

int ji_i18n_format_number(const JiI18nEngine* engine, double value, char* out, int out_size)
{
    if (!out || out_size <= 0) return -1;
    const char* dec = ".";
    const char* sep = "";
    if (engine && engine->current_locale >= 0) {
        const JiLocale* loc = engine->locales[engine->current_locale];
        if (loc->decimal_point[0]) dec = loc->decimal_point;
        if (loc->thousands_sep[0]) sep = loc->thousands_sep;
    }
    /* Format integer part with thousands separators */
    int64_t int_part = (int64_t)value;
    double frac_part = value - (double)int_part;
    if (frac_part < 0) frac_part = -frac_part;
    if (int_part < 0) {
        out[0] = '-';
        out[1] = '\0';
        int_part = -int_part;
    } else {
        out[0] = '\0';
    }
    /* Build integer string in reverse */
    char rev[64];
    int ri = 0;
    int count = 0;
    if (int_part == 0) {
        rev[ri++] = '0';
    }
    while (int_part > 0 && ri < 63) {
        rev[ri++] = '0' + (int)(int_part % 10);
        int_part /= 10;
        count++;
        if (count % 3 == 0 && int_part > 0 && sep[0]) {
            rev[ri++] = sep[0];
        }
    }
    /* Append reversed integer to output */
    int oi = (int)strlen(out);
    for (int i = ri - 1; i >= 0 && oi < out_size - 1; i--) {
        out[oi++] = rev[i];
    }
    /* Fractional part */
    if (frac_part > 0.0) {
        if (oi < out_size - 1) out[oi++] = dec[0];
        for (int i = 0; i < 6 && frac_part > 0.0 && oi < out_size - 1; i++) {
            frac_part *= 10.0;
            int digit = (int)frac_part;
            out[oi++] = '0' + digit;
            frac_part -= digit;
        }
    }
    out[oi] = '\0';
    return oi;
}

int ji_i18n_format_date(const JiI18nEngine* engine, int year, int month, int day,
                          char* out, int out_size)
{
    if (!out || out_size <= 0) return -1;
    const char* fmt = "%Y-%m-%d";
    if (engine && engine->current_locale >= 0) {
        const JiLocale* loc = engine->locales[engine->current_locale];
        if (loc->date_format[0]) fmt = loc->date_format;
    }
    /* Simple format substitution */
    int oi = 0;
    for (const char* p = fmt; *p && oi < out_size - 1; p++) {
        if (*p == '%' && p[1]) {
            p++;
            switch (*p) {
                case 'Y': oi += snprintf(out + oi, out_size - oi, "%04d", year); break;
                case 'm': oi += snprintf(out + oi, out_size - oi, "%02d", month); break;
                case 'd': oi += snprintf(out + oi, out_size - oi, "%02d", day); break;
                default: out[oi++] = *p; break;
            }
        } else {
            out[oi++] = *p;
        }
    }
    out[oi] = '\0';
    return oi;
}

int ji_i18n_format_time(const JiI18nEngine* engine, int hour, int minute, int second,
                          char* out, int out_size)
{
    if (!out || out_size <= 0) return -1;
    const char* fmt = "%H:%M:%S";
    if (engine && engine->current_locale >= 0) {
        const JiLocale* loc = engine->locales[engine->current_locale];
        if (loc->time_format[0]) fmt = loc->time_format;
    }
    int oi = 0;
    for (const char* p = fmt; *p && oi < out_size - 1; p++) {
        if (*p == '%' && p[1]) {
            p++;
            switch (*p) {
                case 'H': oi += snprintf(out + oi, out_size - oi, "%02d", hour); break;
                case 'M': oi += snprintf(out + oi, out_size - oi, "%02d", minute); break;
                case 'S': oi += snprintf(out + oi, out_size - oi, "%02d", second); break;
                default: out[oi++] = *p; break;
            }
        } else {
            out[oi++] = *p;
        }
    }
    out[oi] = '\0';
    return oi;
}

/* =========================================================================
 * Built-in Locales
 * ========================================================================= */

void ji_i18n_locale_en(JiLocale* locale)
{
    memset(locale, 0, sizeof(*locale));
    strcpy(locale->language, "en");
    strcpy(locale->country, "US");
    locale->direction = JI_TEXT_DIR_LTR;
    strcpy(locale->decimal_point, ".");
    strcpy(locale->thousands_sep, ",");
    strcpy(locale->date_format, "%Y-%m-%d");
    strcpy(locale->time_format, "%H:%M:%S");
    locale->first_day_of_week = 0;
}

void ji_i18n_locale_ar(JiLocale* locale)
{
    memset(locale, 0, sizeof(*locale));
    strcpy(locale->language, "ar");
    strcpy(locale->country, "SA");
    locale->direction = JI_TEXT_DIR_RTL;
    strcpy(locale->decimal_point, "\xd9\xab");   /* Arabic decimal separator */
    strcpy(locale->thousands_sep, "\xd9\xac");
    strcpy(locale->date_format, "%Y-%m-%d");
    strcpy(locale->time_format, "%H:%M:%S");
    locale->first_day_of_week = 6;   /* Saturday */
}

void ji_i18n_locale_fr(JiLocale* locale)
{
    memset(locale, 0, sizeof(*locale));
    strcpy(locale->language, "fr");
    strcpy(locale->country, "FR");
    locale->direction = JI_TEXT_DIR_LTR;
    strcpy(locale->decimal_point, ",");
    strcpy(locale->thousands_sep, " ");
    strcpy(locale->date_format, "%d/%m/%Y");
    strcpy(locale->time_format, "%H:%M:%S");
    locale->first_day_of_week = 1;
}

void ji_i18n_locale_de(JiLocale* locale)
{
    memset(locale, 0, sizeof(*locale));
    strcpy(locale->language, "de");
    strcpy(locale->country, "DE");
    locale->direction = JI_TEXT_DIR_LTR;
    strcpy(locale->decimal_point, ",");
    strcpy(locale->thousands_sep, ".");
    strcpy(locale->date_format, "%d.%m.%Y");
    strcpy(locale->time_format, "%H:%M:%S");
    locale->first_day_of_week = 1;
}

void ji_i18n_locale_es(JiLocale* locale)
{
    memset(locale, 0, sizeof(*locale));
    strcpy(locale->language, "es");
    strcpy(locale->country, "ES");
    locale->direction = JI_TEXT_DIR_LTR;
    strcpy(locale->decimal_point, ",");
    strcpy(locale->thousands_sep, ".");
    strcpy(locale->date_format, "%d/%m/%Y");
    strcpy(locale->time_format, "%H:%M:%S");
    locale->first_day_of_week = 1;
}

void ji_i18n_locale_fa(JiLocale* locale)
{
    memset(locale, 0, sizeof(*locale));
    strcpy(locale->language, "fa");
    strcpy(locale->country, "IR");
    locale->direction = JI_TEXT_DIR_RTL;
    strcpy(locale->decimal_point, "\xd9\xab");
    strcpy(locale->thousands_sep, "\xd9\xac");
    strcpy(locale->date_format, "%Y/%m/%d");
    strcpy(locale->time_format, "%H:%M:%S");
    locale->first_day_of_week = 6;
}

void ji_i18n_locale_he(JiLocale* locale)
{
    memset(locale, 0, sizeof(*locale));
    strcpy(locale->language, "he");
    strcpy(locale->country, "IL");
    locale->direction = JI_TEXT_DIR_RTL;
    strcpy(locale->decimal_point, ".");
    strcpy(locale->thousands_sep, ",");
    strcpy(locale->date_format, "%d/%m/%Y");
    strcpy(locale->time_format, "%H:%M:%S");
    locale->first_day_of_week = 0;
}

void ji_i18n_locale_ja(JiLocale* locale)
{
    memset(locale, 0, sizeof(*locale));
    strcpy(locale->language, "ja");
    strcpy(locale->country, "JP");
    locale->direction = JI_TEXT_DIR_LTR;
    strcpy(locale->decimal_point, ".");
    strcpy(locale->thousands_sep, ",");
    strcpy(locale->date_format, "%Y/%m/%d");
    strcpy(locale->time_format, "%H:%M:%S");
    locale->first_day_of_week = 0;
}

void ji_i18n_locale_zh(JiLocale* locale)
{
    memset(locale, 0, sizeof(*locale));
    strcpy(locale->language, "zh");
    strcpy(locale->country, "CN");
    locale->direction = JI_TEXT_DIR_LTR;
    strcpy(locale->decimal_point, ".");
    strcpy(locale->thousands_sep, ",");
    strcpy(locale->date_format, "%Y/%m/%d");
    strcpy(locale->time_format, "%H:%M:%S");
    locale->first_day_of_week = 1;
}

void ji_i18n_locale_ru(JiLocale* locale)
{
    memset(locale, 0, sizeof(*locale));
    strcpy(locale->language, "ru");
    strcpy(locale->country, "RU");
    locale->direction = JI_TEXT_DIR_LTR;
    strcpy(locale->decimal_point, ",");
    strcpy(locale->thousands_sep, " ");
    strcpy(locale->date_format, "%d.%m.%Y");
    strcpy(locale->time_format, "%H:%M:%S");
    locale->first_day_of_week = 1;
}
