#ifndef _PORTAUDIO_NIF_UTIL_
#define _PORTAUDIO_NIF_UTIL_

#include <stdio.h>
#include <stdlib.h>

/**
 * Mark a parameter as unused.
 */
#define unused(x) (void)(x)

/**
 * Log a message to `stderr`, flushing it to the console immediately.
 */
#define log_error(fmt, ...)                     \
        do {                                    \
                fprintf(stderr, (fmt), ##__VA_ARGS__);  \
                fflush(stderr); \
        } while(0)

/**
 * Ensure that the given expression is truthy, printing an error message
 * and aborting otherwise.
 */
#define ensure(expr)                                                    \
        if (!(expr)) {                                                  \
                log_error("CRITICAL ERROR: System failure: %s %s:%s:%d", \
                          #expr, __FUNCTION__, __FILE__, __LINE__);     \
                abort();                                                \
        }

#endif // _PORTAUDIO_NIF_UTIL_
