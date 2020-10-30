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
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern pthread_mutex_t s_sig_lock2;

// fflush necessary for printing through pipes
#define LOG(level, fmt, ...)                                                  \
  do {                                                                        \
    if ((level) >= LOG_LEVEL_VERBOSITY) {                                     \
      pthread_mutex_lock(&s_sig_lock2);                                       \
      printf("[%u] %s:%u: " fmt, (level), __FILE__, __LINE__, ##__VA_ARGS__); \
      fflush(stdout);                                                         \
      pthread_mutex_lock(&s_sig_lock2);                                       \
      pthread_mutex_unlock(&s_sig_lock2);                                     \
    }                                                                         \
  } while (0)
#endif
