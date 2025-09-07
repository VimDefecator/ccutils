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
    substr_t ss;
    ss.p = s;
    for (ss.len = 0; ss.len < n; ++ss.len) {
        if (!s[ss.len]) {
            break;
        }
    }
    return ss;
}

static inline substr_t substr_split_get_head(substr_t ss, char sep)
{
    size_t len;
    for (len = 0; len < ss.len; ++len) {
        if (ss.p[len] == sep) {
            break;
        }
    }
    ss.len = len;
    return ss;
}

static inline void substr_split(substr_t ss, char sep, substr_t *head, substr_t *tail)
{
    *head = substr_split_get_head(ss, sep);
    if (head->len < ss.len) {
        tail->p = ss.p + head->len + 1;
        tail->len = ss.len - head->len - 1;
    } else {
        *tail = EMPTY_SUBSTR;
    }
}

static inline substr_t substr_split_anysep_get_head(substr_t ss, const char *seps)
{
    size_t len;
    for (len = 0; len < ss.len; ++len) {
        if (strchr(seps, ss.p[len])) {
            break;
        }
    }
    ss.len = len;
    return ss;
}

static inline char substr_split_anysep(substr_t ss, const char *seps, substr_t *head, substr_t *tail)
{
    *head = substr_split_anysep_get_head(ss, seps);
    if (head->len < ss.len) {
        tail->p = ss.p + head->len + 1;
        tail->len = ss.len - head->len - 1;
        return tail->p[-1];
    } else {
        *tail = EMPTY_SUBSTR;
        return '\0';
    }
}

static inline substr_t str_split_get_head(const char *s, char sep)
{
    substr_t ss;
    ss.p = s;
    for (ss.len = 0; s[ss.len]; ++ss.len) {
        if (s[ss.len] == sep) {
            break;
        }
    }
    return ss;
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

#define foreach_substr_split_anysep(ss, ss_parent, seps) \
    for(substr_t ss, _tail = ss_parent; substr_split_anysep(_tail, seps, &ss, &_tail), ss.p;)

#define foreach_str_split_anysep(ss, str, seps) \
    foreach_substr_split_anysep(ss, str2substr(str), seps)

#define foreach_substr_split(ss, ss_parent, sep) \
    for(substr_t ss, _tail = ss_parent; substr_split(_tail, sep, &ss, &_tail), ss.p;)

#define foreach_str_split(ss, str, sep) \
    foreach_substr_split(ss, str2substr(str), sep)

#endif
