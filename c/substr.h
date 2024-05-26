#ifndef INCLUDE_SUBSTR_H
#define INCLUDE_SUBSTR_H

#include <stddef.h>
#include <string.h>

typedef struct
{
  const char *p;
  size_t len, lenMore;
}
SubStr;

static inline SubStr str2substr(const char *s)
{
  SubStr ss;
  ss.p = s;
  ss.len = ss.lenMore = strlen(s);

  return ss;
}

static inline size_t substr_getlen(SubStr ss, char sep)
{
  const char *pNext = (const char *)memchr(ss.p, sep, ss.lenMore);

  return pNext ? pNext - ss.p : ss.lenMore;
}

static inline SubStr substr_substr(SubStr ss, char sep)
{
  ss.lenMore = ss.len;

  if(ss.lenMore != 0)
    ss.len = substr_getlen(ss, sep);
  else
    ss.p = NULL;

  return ss;
}

static inline SubStr substr(const char *s, char sep)
{
  return substr_substr(str2substr(s), sep);
}

static inline SubStr substr_next(SubStr ss, char sep)
{
  if(ss.p)
  {
    if(ss.len < ss.lenMore)
    {
      ss.p += ss.len + 1;
      ss.lenMore -= ss.len + 1;
    }
    else
    {
      ss.p += ss.lenMore;
      ss.lenMore = 0;
    }
    
    if(ss.lenMore != 0)
    {
      ss.len = substr_getlen(ss, sep);
    }
    else
    {
      ss.p = NULL;
      ss.len = 0;
    }
  }

  return ss;
}

static inline int substr_eq_str(SubStr ss, const char *s)
{
  size_t len = strlen(s);

  return ss.len == len && !memcmp(ss.p, s, len);
}

#define foreach_substr(ss,str,sep) for(SubStr ss = substr(str, sep); ss.p; ss = substr_next(ss, sep))

#endif
