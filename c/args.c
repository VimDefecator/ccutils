#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "args.h"
#include "substr.h"

static int isKeyValueArgName(const char *argName, const char *fmt)
{
  foreach_substr(ss_arg, fmt, ';')
  {
    SubStr ss_name, ss_type;
    ss_name = substr_substr(ss_arg, '%');
    ss_type = substr_next(ss_name, '%');

    if(substr_eq_str(ss_name, argName) && ss_type.p)
      return 1;
  }

  return 0;
}

void argscanf(char **argv, const char *fmt, ...)
{
  va_list va;

  va_start(va, fmt);

  foreach_substr(ss_arg, fmt, ';')
  {
    SubStr ss_name, ss_type;
    char **argp;

    ss_name = substr_substr(ss_arg, '%');
    ss_type = substr_next(ss_name, '%');

    for(argp = argv + 1; *argp; ++argp)
      if(argp[0][0] == '-' && substr_eq_str(ss_name, &argp[0][1]))
        if(!(argp[-1][0] == '-' && isKeyValueArgName(&argp[-1][1], fmt)))
          break;

    if(*argp && ss_type.p)
      ++argp;

    if(ss_type.p == NULL)
    {
      int *dst = va_arg(va, int *);
      if(*argp)
        *dst = 1;
    }
    else if(substr_eq_str(ss_type, "d"))
    {
      int *dst = va_arg(va, int *);
      if(*argp)
        *dst = strtol(*argp, NULL, 10);
    }
    else if(substr_eq_str(ss_type, "ld"))
    {
      long *dst = va_arg(va, long *);
      if(*argp)
        *dst = strtol(*argp, NULL, 10);
    }
    else if(substr_eq_str(ss_type, "f"))
    {
      float *dst = va_arg(va, float *);
      if(*argp)
        *dst = strtof(*argp, NULL);
    }
    else if(substr_eq_str(ss_type, "lf"))
    {
      double *dst = va_arg(va, double *);
      if(*argp)
        *dst = strtof(*argp, NULL);
    }
    else if(substr_eq_str(ss_type, "s"))
    {
      char *dst = va_arg(va, char *);
      if(*argp)
        strcpy(dst, *argp);
    }
  }

  va_end(va);
}
