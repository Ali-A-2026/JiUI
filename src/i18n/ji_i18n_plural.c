/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_i18n_plural.c
 * @brief CLDR plural rules implementation for various languages.
 */

#include "jiui/ji_i18n.h"
#include <string.h>
#include <stdlib.h>

/* English/German/Spanish/Dutch/Swedish/Danish/Norwegian: one vs other */
static JiPluralCategory plural_germanic(int64_t count)
{
    if (count == 1) return JI_PLURAL_ONE;
    return JI_PLURAL_OTHER;
}

/* French: 0-1 → one, else other */
static JiPluralCategory plural_french(int64_t count)
{
    if (count == 0 || count == 1) return JI_PLURAL_ONE;
    return JI_PLURAL_OTHER;
}

/* Russian: complex rules based on last two digits */
static JiPluralCategory plural_russian(int64_t count)
{
    int64_t abs = count < 0 ? -count : count;
    int mod10 = (int)(abs % 10);
    int mod100 = (int)(abs % 100);
    if (mod10 == 1 && mod100 != 11) return JI_PLURAL_ONE;
    if (mod10 >= 2 && mod10 <= 4 && (mod100 < 10 || mod100 >= 20)) return JI_PLURAL_FEW;
    return JI_PLURAL_MANY;
}

/* Arabic: zero, one, two, few, many, other */
static JiPluralCategory plural_arabic(int64_t count)
{
    int64_t abs = count < 0 ? -count : count;
    int mod100 = (int)(abs % 100);
    if (count == 0) return JI_PLURAL_ZERO;
    if (count == 1) return JI_PLURAL_ONE;
    if (count == 2) return JI_PLURAL_TWO;
    if (mod100 >= 3 && mod100 <= 10) return JI_PLURAL_FEW;
    if (mod100 >= 11) return JI_PLURAL_MANY;
    return JI_PLURAL_OTHER;
}

/* Polish: one, few, many */
static JiPluralCategory plural_polish(int64_t count)
{
    int64_t abs = count < 0 ? -count : count;
    int mod10 = (int)(abs % 10);
    int mod100 = (int)(abs % 100);
    if (count == 1) return JI_PLURAL_ONE;
    if (mod10 >= 2 && mod10 <= 4 && (mod100 < 10 || mod100 >= 20)) return JI_PLURAL_FEW;
    return JI_PLURAL_MANY;
}

/* Japanese/Korean/Chinese/Thai/Vietnamese: no plural distinction */
static JiPluralCategory plural_asian(int64_t count)
{
    (void)count;
    return JI_PLURAL_OTHER;
}

JiPluralCategory ji_i18n_plural_rule(const char* lang_code, int64_t count)
{
    if (!lang_code) return plural_germanic(count);

    if (strcmp(lang_code, "en") == 0 || strcmp(lang_code, "de") == 0 ||
        strcmp(lang_code, "es") == 0 || strcmp(lang_code, "nl") == 0 ||
        strcmp(lang_code, "sv") == 0 || strcmp(lang_code, "da") == 0 ||
        strcmp(lang_code, "no") == 0 || strcmp(lang_code, "it") == 0 ||
        strcmp(lang_code, "pt") == 0 || strcmp(lang_code, "el") == 0) {
        return plural_germanic(count);
    }
    if (strcmp(lang_code, "fr") == 0) return plural_french(count);
    if (strcmp(lang_code, "ru") == 0 || strcmp(lang_code, "uk") == 0 ||
        strcmp(lang_code, "hr") == 0 || strcmp(lang_code, "sr") == 0 ||
        strcmp(lang_code, "cs") == 0 || strcmp(lang_code, "sk") == 0) {
        return plural_russian(count);
    }
    if (strcmp(lang_code, "ar") == 0) return plural_arabic(count);
    if (strcmp(lang_code, "pl") == 0) return plural_polish(count);
    if (strcmp(lang_code, "ja") == 0 || strcmp(lang_code, "ko") == 0 ||
        strcmp(lang_code, "zh") == 0 || strcmp(lang_code, "th") == 0 ||
        strcmp(lang_code, "vi") == 0 || strcmp(lang_code, "id") == 0) {
        return plural_asian(count);
    }
    /* Default: Germanic rules */
    return plural_germanic(count);
}
