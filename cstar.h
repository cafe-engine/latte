#ifndef CSTAR_H
#define CSTAR_H

#define CS_VERSION "0.1.0"

#ifndef CS_API
    #if defined(_WIN32)
        #if defined(BUILD_SHARE)
	    #define TIC_API __declspec(dllexport)
        #elif defined(USE_SHARED)
            #define TIC_API __declspec(dllimport)
        #else
            #define TIC_API
        #endif
    #else
        #define CS_API
    #endif
#endif

#define CS_STR(x) #x
#define CS_ASSERT(s) if (!(s)) {CS_TRACELOG("Assertion '%s' failed", CS_STR(s)); exit(-1);}

#define CS_TRACEERR(message...) cst_tracelog(1, __FILE__, __PRETTY_FUNCTION__, __LINE__, message)
#define CS_ERRLOG(message...) cst_log(1, message)

#define CS_TRACELOG(message...) cst_tracelog(0, __FILE__, __PRETTY_FUNCTION__, __LINE__, message)
#define CS_LOG(message...) cst_log(0, message)

enum {
    CS_LOG_MODE = 0,
    CS_ERROR_MODE,
    CS_WARNING_MODE
};

CS_API void cst_log(int mode, const char *fmt, ...); 
CS_API void cst_tracelog(int mode, const char *file, const char *function, int line, const char *fmt, ...);

#endif /* CSTAR_H */

#if defined(CSTAR_IMPLEMENTATION)

#include <stdio.h>
#include <time.h>
#include <stdarg.h>

void cst_log(int mode, const char *fmt, ...) {
  time_t t = time(NULL);
  struct tm *tm_now;

  va_list args;

  char err[15] = "";
  /*if (type == 1) sprintf((char*)err, "ERROR: ");*/
  switch (mode) {
    case CS_LOG_MODE: sprintf((char*)err, "[log] "); break;
    case CS_ERROR_MODE: sprintf((char*)err, "[error] "); break;
    case CS_WARNING_MODE: sprintf((char*)err, "[warning] "); break;
  }

  tm_now = localtime(&t);
  char buf[10];
  strftime((char*)buf, sizeof(buf), "%H:%M:%S", tm_now);
  fprintf(stderr, "%s %s", buf, err);
  va_start(args, fmt);
  vfprintf(stderr, (const char*)fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}

void cst_tracelog(int mode, const char *file, const char *function, int line, const char *fmt, ...) {
  time_t t = time(NULL);
  struct tm *tm_now;

  va_list args;

  char err[15] = "";
  /* if (mode == 1) sprintf((char*)err, "ERROR in "); */
  switch (mode) {
    case CS_LOG_MODE: sprintf((char*)err, "[tracelog] "); break;
    case CS_ERROR_MODE: sprintf((char*)err, "[error] "); break;
    case CS_WARNING_MODE: sprintf((char*)err, "[warning] "); break;
  }

  tm_now = localtime(&t);
  char buf[10];
  strftime((char*)buf, sizeof(buf), "%H:%M:%S", tm_now);
  fprintf(stderr, "%s %s:%d %s%s(): ", buf, file, line, err, function);

  va_start(args, fmt);
  vfprintf(stderr, (const char*)fmt, args);

  va_end(args);
  fprintf(stderr, "\n");
}

#if defined(CSTAR_CLI)
int main(int argc, char ** argv) {

    return 0;
}
#endif /* CSTAR_CLI */

#endif /* CSTAR_IMPLEMENTATION */
