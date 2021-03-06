#include <time.h>
#include "features.h"

hidden int __days_in_month(int, int);
hidden int __month_to_secs(int, int);
hidden long long __year_to_secs(long long, int *);
hidden long long __tm_to_secs(const struct tm *);
hidden const char *__tm_to_tzname(const struct tm *);
hidden int __secs_to_tm(long long, struct tm *);
hidden void __secs_to_zone(long long, int, int *, long *, long *, const char **);
hidden const char *__strftime_fmt_1(char (*)[100], size_t *, int, const struct tm *, locale_t, int);
extern hidden const char __utc[];

hidden size_t __strftime_l(char *restrict, size_t, const char *restrict, const struct tm *restrict, locale_t);
