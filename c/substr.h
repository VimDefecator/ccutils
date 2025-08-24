#ifndef INCLUDE_SUBSTR_H
#define INCLUDE_SUBSTR_H

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

typedef struct {
    const char *p;
    size_t len;
} substr_t;

static const substr_t EMPTY_SUBSTR = {};

static inline substr_t str2substr(const char *s)
{
    substr_t ss = {};
    if (s) {
        ss.p = s;
        ss.len = strlen(s);
    }
    return ss;
}

static inline substr_t str2substr_n(const char *s, size_t n)
{
    substr_t ss = str2substr(s);
    if (ss.len > n) {
        ss.len = n;
    }
    return ss;
}

static inline size_t substr_getlen(substr_t ss, char sep)
{
    size_t i;
    for (i = 0; i < ss.len; ++i) {
        if (ss.p[i] == sep) {
            break;
        }
    }
    return i;
}

static inline void substr_split(substr_t ss, char sep, substr_t *head, substr_t *tail)
{
    size_t len = substr_getlen(ss, sep);
    head->p = ss.p;
    head->len = len;
    if (len < ss.len) {
        tail->p = ss.p + len + 1;
        tail->len = ss.len - len - 1;
    } else {
        *tail = EMPTY_SUBSTR;
    }
}

static inline void substr_split_many(substr_t ss, char sep, int n, ...)
{
    va_list va;
    va_start(va, n);

    for (; n > 0; --n) {
        substr_t *dst = va_arg(va, substr_t *);
        substr_split(ss, sep, dst, &ss);
    }

    va_end(va);
}

static inline bool substr_eq(substr_t ss1, substr_t ss2)
{
    return ss1.len == ss2.len && (ss1.len == 0 || !memcmp(ss1.p, ss2.p, ss1.len));
}

static inline bool substr_eq_str(substr_t ss, const char *s)
{
    return substr_eq(ss, str2substr(s));
}

#define foreach_substr(ss,str,sep) for(substr_t ss, _tail = str2substr(str); substr_split(_tail, sep, &ss, &_tail), ss.p;)

#endif
