#pragma once
// General logging functions.
// Logging is done via printf which retargets to UART by default on stm32f0xx.
//
// Best practice is to use log level WARNING for recoverable faults and CRITICAL
// for irrecoverable faults. DEBUG can be used for anything.
#include <stdio.h>

typedef enum {
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_WARN,
  LOG_LEVEL_CRITICAL,
  NUM_LOG_LEVELS,
} LogLevel;

#define LOG_LEVEL_NONE NUM_LOG_LEVELS

#ifndef LOG_LEVEL_VERBOSITY
#define LOG_LEVEL_VERBOSITY LOG_LEVEL_DEBUG
#endif

#define LOG_DEBUG(fmt, ...) LOG(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) LOG(LOG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__)

#ifndef MPXE
#define LOG(level, fmt, ...)                                                  \
  do {                                                                        \
    if ((level) >= LOG_LEVEL_VERBOSITY) {                                     \
      printf("[%u] %s:%u: " fmt, (level), __FILE__, __LINE__, ##__VA_ARGS__); \
    }                                                                         \
  } while (0)
#else
#include <stdbool.h>
#include <string.h>

#include "store.h"

#define MAX_LOG_LEN 256
static char s_log_buf[MAX_LOG_LEN];

// fflush necessary for printing through pipes
#define LOG(level, fmt, ...)                                                                  \
  do {                                                                                        \
    if ((level) >= LOG_LEVEL_VERBOSITY) {                                                     \
      memset(s_log_buf, 0, sizeof(s_log_buf));                                                \
      int len = snprintf(s_log_buf, sizeof(s_log_buf), "[%u] %s:%u: " fmt, (level), __FILE__, \
                         __LINE__, ##__VA_ARGS__);                                            \
      log_export(s_log_buf, len);                                                             \
    }                                                                                         \
  } while (0)
#endif
