// vim:ts=4:sw=4:expandtab
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <locale.h>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_version.h>

#include "i3status.h"

static bool local_timezone_init = false;
static const char *local_timezone = NULL;
static const char *current_timezone = NULL;

void set_timezone(const char *tz) {
    if (!local_timezone_init) {
        /* First call, initialize. */
        local_timezone = getenv("TZ");
        local_timezone_init = true;
    }
    if (tz == NULL || tz[0] == '\0') {
        /* User wants localtime. */
        tz = local_timezone;
    }
    if (tz != current_timezone) {
        if (tz) {
            setenv("TZ", tz, 1);
        } else {
            unsetenv("TZ");
        }
        current_timezone = tz;
    }
    tzset();
}

void print_time(yajl_gen json_gen, char *buffer, const char *title, const char *format, const char *tz, const char *locale, const char *format_time, time_t t, int interval, int update_interval) {
    const char *walk;
    char *outwalk = buffer;
    struct tm tm;
    static char timebuf[1024];

    if (title != NULL)
        INSTANCE(title);

    set_timezone(tz);
    localtime_r(&t, &tm);

    if (locale != NULL) {
        setlocale(LC_ALL, locale);
    }

    if (format_time == NULL) {
        if ((tm.tm_sec % update_interval) < interval || timebuf[0] == '\0') {
            strftime(timebuf, sizeof(timebuf), format, &tm);
        }
        maybe_escape_markup(timebuf, &outwalk);
    } else {
        for (walk = format; *walk != '\0';) {
            if (*walk != '%') {
                *(outwalk++) = *walk++;

            } else if (BEGINS_WITH(walk + 1, "time")) {
                if ((tm.tm_sec % update_interval) < interval || timebuf[0] == '\0') {
                    strftime(timebuf, sizeof(timebuf), format_time, &tm);
                }
                maybe_escape_markup(timebuf, &outwalk);
                walk += sizeof("time");

            } else {
                *(outwalk++) = '%';
                ++walk;
            }
        }
    }

    if (locale != NULL) {
        setlocale(LC_ALL, "");
    }

    *outwalk = '\0';
    OUTPUT_FULL_TEXT(buffer);
}
