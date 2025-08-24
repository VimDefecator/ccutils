#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "args.h"
#include "substr.h"

enum {
    DST_TYPE_FLAG,
    DST_TYPE_INT,
    DST_TYPE_LONG,
    DST_TYPE_FLOAT,
    DST_TYPE_DOUBLE,
    DST_TYPE_STRING,
};

enum {
    DST_PTR_TYPE_DIRECT,
    DST_PTR_TYPE_ARRAY,
    DST_PTR_TYPE_CALLBACK,
    DST_PTR_TYPE_SKIP,
};

typedef struct {
    substr_t ss_name_short, ss_name_long;
    int type, ptr_type, count;
} argdef_t;

static bool is_short_arg_name(substr_t ss)
{
    return ss.len == 2 && ss.p[0] == '-' && ss.p[1] != '-';
}

static bool is_long_arg_name(substr_t ss)
{
    return ss.len > 2 && ss.p[0] == '-' && ss.p[1] == '-' && ss.p[2] != '-';
}

static bool parse_argdef(argdef_t *def, substr_t ss_argdef)
{
    substr_t ss1, ss2, ss3;
    substr_split_many(ss_argdef, ' ', 3, &ss1, &ss2, &ss3);

    substr_t ss_dstdef;

    if (is_short_arg_name(ss1)) {
        def->ss_name_short = ss1;
        if (is_long_arg_name(ss2)) {
            def->ss_name_long = ss2;
            ss_dstdef = ss3;
        } else if (!ss3.p) {
            def->ss_name_long = EMPTY_SUBSTR;
            ss_dstdef = ss2;
        } else {
            return false;
        }
    } else if (is_long_arg_name(ss1)) {
        def->ss_name_long = ss1;
        if (is_short_arg_name(ss2)) {
            def->ss_name_short = ss2;
            ss_dstdef = ss3;
        } else if (!ss3.p) {
            def->ss_name_short = EMPTY_SUBSTR;
            ss_dstdef = ss2;
        } else {
            return false;
        }
    } else if (ss1.p && !ss2.p && !ss3.p) {
        def->ss_name_short = def->ss_name_long = EMPTY_SUBSTR;
        ss_dstdef = ss1;
    } else {
        return false;
    }

    substr_t ss_type, ss_count;
    substr_split(ss_dstdef, '*', &ss_type, &ss_count);

    if (ss_type.len == 0) {
        def->type = DST_TYPE_FLAG;
    } else if (substr_eq_str(ss_type, "d")) {
        def->type = DST_TYPE_INT;
    } else if (substr_eq_str(ss_type, "ld")) {
        def->type = DST_TYPE_LONG;
    } else if (substr_eq_str(ss_type, "f")) {
        def->type = DST_TYPE_FLOAT;
    } else if (substr_eq_str(ss_type, "lf")) {
        def->type = DST_TYPE_DOUBLE;
    } else if (substr_eq_str(ss_type, "s")) {
        def->type = DST_TYPE_STRING;
    } else {
        return false;
    }

    if (ss_count.len > 0) {
        def->count = 0;
        if (substr_eq_str(ss_count, "arr")) {
            def->ptr_type = DST_PTR_TYPE_ARRAY;
        } else if (substr_eq_str(ss_count, "cb")) {
            def->ptr_type = DST_PTR_TYPE_CALLBACK;
        } else if (substr_eq_str(ss_count, "skip")) {
            def->ptr_type = DST_PTR_TYPE_SKIP;
        } else {
            char *end;
            long count = strtol(ss_count.p, &end, 10);
            if (end - ss_count.p == ss_count.len && 1 <= count && count <= 10) {
                def->count = count;
            } else {
                return false;
            }
            def->ptr_type = DST_PTR_TYPE_DIRECT;
        }
    } else {
        def->count = 1;
        def->ptr_type = DST_PTR_TYPE_DIRECT;
    }

    return true;
}

static bool find_and_parse_argdef(argdef_t *def, const char *fmt, substr_t ss_name)
{
    if (is_short_arg_name(ss_name)) {
        foreach_substr(ss_argdef, fmt, '\\') {
            if (!parse_argdef(def, ss_argdef)) {
                break;
            }
            if (substr_eq(def->ss_name_short, ss_name)) {
                return true;
            }
        }
    } else if (is_long_arg_name(ss_name)) {
        foreach_substr(ss_argdef, fmt, '\\') {
            if (!parse_argdef(def, ss_argdef)) {
                break;
            }
            if (substr_eq(def->ss_name_long, ss_name)) {
                return true;
            }
        }
    }
    return false;
}

static bool has_argdef(const char *fmt, const char *name)
{
    argdef_t def;

    if (find_and_parse_argdef(&def, fmt, str2substr(name))) {
        return true;
    }
    if (find_and_parse_argdef(&def, fmt, str2substr_n(name, 2))) {
        return true;
    }
    return false;
}

static bool has_key_value_argdef(const char *fmt, const char *name)
{
    argdef_t def;

    return find_and_parse_argdef(&def, fmt, str2substr(name)) && def.type != DST_TYPE_FLAG;
}

static bool is_last_arg_key_without_value(const char *fmt, char **first, char **last)
{
    if (first < last) {
        return has_key_value_argdef(fmt, *last) && !is_last_arg_key_without_value(fmt, first, last - 1);
    } else {
        return has_key_value_argdef(fmt, *last);
    }
}

static const char *find_arg(char **argv, const argdef_t *def, const char *fmt, char ***remaining_args_ptr)
{
    char *arg = NULL;
    char **argp;
    for (argp = argv; *argp; ++argp) {
        if (argp > argv && is_last_arg_key_without_value(fmt, argv, argp - 1)) {
            continue;
        }
        if (!def->ss_name_short.p && !def->ss_name_long.p) {
            if (!has_argdef(fmt, *argp)) {
                arg = *argp;
                break;
            }
        } else if (def->type == DST_TYPE_FLAG) {
            if (is_short_arg_name(str2substr_n(*argp, 2)) && strchr(*argp + 1, def->ss_name_short.p[1])) {
                arg = *argp;
                break;
            } else if (substr_eq_str(def->ss_name_long, *argp)) {
                arg = *argp;
                break;
            }
        } else {
            if (substr_eq(def->ss_name_short, str2substr_n(*argp, 2))) {
                if (strlen(*argp) > 2) {
                    arg = *argp + 2;
                } else {
                    arg = *++argp;
                }
                break;
            } else if (substr_eq_str(def->ss_name_long, *argp)) {
                arg = *++argp;
                break;
            }
        }
    }
    *remaining_args_ptr = argp + !!*argp;

    return arg;
}

bool argscanf(char **argv, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    foreach_substr(ss_argdef, fmt, '\\') {
        argdef_t def;
        if (!parse_argdef(&def, ss_argdef)) {
            return false;
        }

        if (def.ptr_type == DST_PTR_TYPE_DIRECT) {
            for (char **subargs = argv; def.count > 0; --def.count) {
                const char *arg = find_arg(subargs, &def, fmt, &subargs);

                switch (def.type) {
                    case DST_TYPE_FLAG: {
                        bool *dst = va_arg(va, bool *);
                        *dst = !!arg;
                    }
                    break;
                    case DST_TYPE_INT: {
                        int *dst = va_arg(va, int *);
                        if (arg) {
                            *dst = strtol(arg, NULL, 10);
                        } else {
                            *dst = 0;
                        }
                    }
                    break;
                    case DST_TYPE_LONG: {
                        long *dst = va_arg(va, long *);
                        if (arg) {
                            *dst = strtol(arg, NULL, 10);
                        } else {
                            *dst = 0;
                        }
                    }
                    break;
                    case DST_TYPE_FLOAT: {
                        float *dst = va_arg(va, float *);
                        if (arg) {
                            *dst = strtod(arg, NULL);
                        } else {
                            *dst = 0.;
                        }
                    }
                    break;
                    case DST_TYPE_DOUBLE: {
                        double *dst = va_arg(va, double *);
                        if (arg) {
                            *dst = strtod(arg, NULL);
                        } else {
                            *dst = 0;
                        }
                    }
                    break;
                    case DST_TYPE_STRING: {
                        const char **dst = va_arg(va, const char **);
                        *dst = arg;
                    }
                    break;
                }
            }
        } else if (def.ptr_type == DST_PTR_TYPE_ARRAY) {
            void **dstv = NULL;
            int limit = 0;
            if (def.type != DST_TYPE_FLAG) {
                dstv = va_arg(va, void **);
                limit = va_arg(va, int);
            }
            int *count = va_arg(va, int *);

            char **argp = argv;
            int i = 0;
            for (const char *arg; (arg = find_arg(argp, &def, fmt, &argp));) {
                if (i > limit) {
                    continue;
                }
                switch (def.type) {
                    case DST_TYPE_FLAG:
                    break;
                    case DST_TYPE_INT:
                        *(int *)dstv[i] = strtol(arg, NULL, 10);
                    break;
                    case DST_TYPE_LONG:
                        *(long *)dstv[i] = strtol(arg, NULL, 10);
                    break;
                    case DST_TYPE_FLOAT:
                        *(float *)dstv[i] = strtod(arg, NULL);
                    break;
                    case DST_TYPE_DOUBLE:
                        *(double *)dstv[i] = strtod(arg, NULL);
                    break;
                    case DST_TYPE_STRING:
                        strcpy((char *)dstv[i], arg);
                    break;
                }
                ++i;
            }
            *count = i;
        } else if (def.ptr_type == DST_PTR_TYPE_CALLBACK) {
            void *cb = va_arg(va, void *);
            void *ctx = va_arg(va, void *);

            char **argp = argv;
            for (const char *arg; (arg = find_arg(argp, &def, fmt, &argp));) {
                switch (def.type) {
                    case DST_TYPE_FLAG:
                        ((void (*)(void *))cb)(ctx);
                    break;
                    case DST_TYPE_INT:
                        ((void (*)(int, void *))cb)(strtol(arg, NULL, 10), ctx);
                    break;
                    case DST_TYPE_LONG:
                        ((void (*)(long, void *))cb)(strtol(arg, NULL, 10), ctx);
                    break;
                    case DST_TYPE_FLOAT:
                        ((void (*)(float, void *))cb)(strtod(arg, NULL), ctx);
                    break;
                    case DST_TYPE_DOUBLE:
                        ((void (*)(double, void *))cb)(strtod(arg, NULL), ctx);
                    break;
                    case DST_TYPE_STRING:
                        ((void (*)(const char *, void *))cb)(arg, ctx);
                    break;
                }
            }
        }
    }

    va_end(va);

    return true;
}
