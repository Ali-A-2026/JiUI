/**
 * JiUI - DateTimeEdit implementation
 */

#include <jiui/ji_date_time_edit.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

JiDateTimeEdit* ji_date_time_edit_new(void) {
    JiDateTimeEdit* dte = (JiDateTimeEdit*)ji_calloc(1, sizeof(JiDateTimeEdit));
    if (!dte) { JI_ERROR_LOG("ji_date_time_edit_new: out of memory"); return NULL; }
    dte->value.year = 2026; dte->value.month = 1; dte->value.day = 1;
    dte->value.hour = 0; dte->value.minute = 0; dte->value.second = 0;
    dte->minimum.year = 1900; dte->minimum.month = 1; dte->minimum.day = 1;
    dte->maximum.year = 9999; dte->maximum.month = 12; dte->maximum.day = 31;
    dte->format = JI_DT_FORMAT_DATE_TIME;
    dte->calendar_popup = false;
    dte->display_format = NULL;
    return dte;
}

void ji_date_time_edit_destroy(JiDateTimeEdit* dte) { if (dte) { ji_free(dte->display_format); ji_free(dte); } }

void ji_date_time_edit_set_date_time(JiDateTimeEdit* dte, JiDateTime dt) {
    if (!dte) return;
    dte->value = dt;
}

JiDateTime ji_date_time_edit_get_date_time(const JiDateTimeEdit* dte) {
    return dte ? dte->value : (JiDateTime){0, 1, 1, 0, 0, 0};
}

void ji_date_time_edit_set_format(JiDateTimeEdit* dte, JiDateTimeFormat format) { if (dte) dte->format = format; }
void ji_date_time_edit_set_calendar_popup(JiDateTimeEdit* dte, bool popup) { if (dte) dte->calendar_popup = popup; }
