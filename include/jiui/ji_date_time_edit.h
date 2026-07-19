/**
 * JiUI - DateTimeEdit widget header
 */

#ifndef JIUI_DATE_TIME_EDIT_H
#define JIUI_DATE_TIME_EDIT_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiDateTime {
    int year;
    int month;    /* 1-12 */
    int day;      /* 1-31 */
    int hour;     /* 0-23 */
    int minute;   /* 0-59 */
    int second;   /* 0-59 */
} JiDateTime;

typedef enum JiDateTimeFormat {
    JI_DT_FORMAT_DATE_TIME = 0,
    JI_DT_FORMAT_DATE_ONLY = 1,
    JI_DT_FORMAT_TIME_ONLY = 2
} JiDateTimeFormat;

typedef struct JiDateTimeEdit {
    JiControl         control;
    JiDateTime        value;
    JiDateTime        minimum;
    JiDateTime        maximum;
    JiDateTimeFormat  format;
    bool              calendar_popup;
    char*             display_format;  /* e.g. "yyyy-MM-dd HH:mm:ss" */
} JiDateTimeEdit;

JI_API JiDateTimeEdit* ji_date_time_edit_new(void);
JI_API void ji_date_time_edit_destroy(JiDateTimeEdit* dte);
JI_API void ji_date_time_edit_set_date_time(JiDateTimeEdit* dte, JiDateTime dt);
JI_API JiDateTime ji_date_time_edit_get_date_time(const JiDateTimeEdit* dte);
JI_API void ji_date_time_edit_set_format(JiDateTimeEdit* dte, JiDateTimeFormat format);
JI_API void ji_date_time_edit_set_calendar_popup(JiDateTimeEdit* dte, bool popup);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_DATE_TIME_EDIT_H */
