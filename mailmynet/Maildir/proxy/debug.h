/******************************************************************************

 @File Name : {PROJECT_DIR}/debug.h

 @Creation Date : 30-01-2012

 @Last Modified : Tue 07 Feb 2012 10:46:54 PM CST

 @Created By: Wenxuan Zhou

 @Purpose :

*******************************************************************************/
#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _DEBUG

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define debug(str, ...)       {                                           \
  fprintf(stderr, "DEBUG: %s-%d:"str, __FILE__, __LINE__, ##__VA_ARGS__); \
}

#define thread_debug(str, ...) {                                          \
  struct timeval stamp;                                                   \
  gettimeofday(&stamp, NULL);                                             \
  uint64_t stamp_val = stamp.tv_sec * 1000000 + stamp.tv_usec;            \
  fprintf(stderr, "DEBUG %lu: %lu: %s-%d:"str, pthread_self(),            \
                                               stamp_val,                 \
                                               __FILE__,                  \
                                               __LINE__,                  \
                                               ##__VA_ARGS__              \
                                              );                          \
}

#define debug_if(cond, fmt, ...)  \
  if (cond) debug(fmt, ##__VA_ARGS__)

#define thread_debug_if(cond, fmt, ...) \
  if (cond) thread_debug(fmt, __VA_ARGS__)

#define err(str, ...)         {                                   \
  fprintf(stderr, "Error %s-%d: "str,                                 \
      __FILE__, __LINE__, ##__VA_ARGS__);                         \
}

#define err_ptr(str, reason, ...)         {                               \
  fprintf(stderr, "Error %s-%d: "str,                                 \
      __FILE__, __LINE__, ##__VA_ARGS__);                         \
}

#define bug_if(cond, ret_val) {       \
  assert(!(cond));                    \
}

#define chk_expr(expr, val, op)  {                \
  if ((expr) op (val))                            \
    debug("%s %s %s fails\n", #expr, #op, #val);  \
}

#define dump_type_array(addr, len, print_indicator) {     \
  int i;                                                  \
  for (i = 0; i < len; ++i) {                             \
    if (i % 8 == 0)                                       \
      fprintf(stderr, "\n");                              \
    fprintf(stderr, print_indicator" ", addr[i]);         \
  }                                                       \
}

#define dump_array(addr, len, type, print_indicator) {    \
  type* base_addr = (type*) addr;                         \
  dump_type_array(base_addr, len, print_indicator);       \
}

#else

#define assert(x)                     0
#define debug(x, ...)                 0
#define debug_if(x, y, ...)           0
#define thread_debug(x, ...)          0
#define thread_debug_if(x, y, ...)    0
#define dump_array(x,y,z)             0
#define bug_if(cond, ret_val) {                           \
  if (cond) {                                             \
    fprintf(stderr, "BUG: '%s' failed to hold\n", #cond); \
  }                                                       \
}

#define err(str, ...)         {                          \
  fprintf(stderr, "Error: "str, ##__VA_ARGS__);          \
}

#endif /* ifdef _DEBUG */

#define ASSERT(x)                       assert(x)
#define BUG_IF(cond, ret_val)           bug_if(cond, ret_val)
#define DEBUG(str, ...)                 debug(str, ##__VA_ARGS__)
#define DEBUG_IF(cond, str, ...)        debug_if(cond, str, ##__VA_ARGS__)
#define THREAD_DEBUG(str, ...)          thread_debug(str, ##__VA_ARGS__)
#define THREAD_DEBUG_IF(cond, str, ...) thread_debug_if(cond, str, ##__VA_ARGS__)
#define ERR(str, ...)                   err(str,  ##__VA_ARGS__)
#define ERR_PTR(str, ...)               err(str, ##__VA_ARGS__)
#define CHK_EQ(expr, val)               chk_expr(expr, val, !=)
#define CHK_GT(expr, val)               chk_expr(expr, val, <=)
#define CHK_GE(expr, val)               chk_expr(expr, val, <)
#define CHK_LE(expr, val)               chk_expr(expr, val, >)

//#undef __cplusplus
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#define DUMP_BYTE(addr, len)            dump_array(addr, len, char,    "%x")
#define DUMP_DWORD(addr, len)           dump_array(addr, len, int,     "%x")
#define DUMP_INT32_ARRAY(addr, len)     dump_array(addr, len, int32_t, "%"PRId32)
#define DUMP_INT64_ARRAY(addr, len)     dump_array(addr, len, int64_t, "%"PRId64)
#define SYS_ERR_STR()                   strerror(errno)
#define __cplusplus 1

#ifdef __cplusplus
}
#endif
#endif
