#pragma once
#include <stdint.h>

#define CR_BASE_ADDR 0x00
#define TIME_BASE_ADDR 0x03
#define ALARM_BASE_ADDR 0X0A

#define CR ((CrRegDef *)CR_BASE_ADDR)
#define TIME ((TimeRegDef *)TIME_BASE_ADDR)
#define ALARM ((AlarmRegDef *)ALARM_BASE_ADDR)

typedef struct {
  volatile uint8_t CR1;
  volatile uint8_t CR2;
  volatile uint8_t CR3;
} CrRegDef;

typedef struct {
  volatile uint8_t SECONDS;
  volatile uint8_t MINUTES;
  volatile uint8_t HOURS;
  volatile uint8_t DAYS;
  volatile uint8_t WEEKDAYS;
  volatile uint8_t MONTHS;
  volatile uint8_t YEARS;
} TimeRegDef;

typedef struct {
  volatile uint8_t MINUTE_ALARM;
  volatile uint8_t HOUR_ALARM;
  volatile uint8_t DAY_ALARM;
  volatile uint8_t WEEKDAY_ALARM;
  volatile uint8_t OFFSET;
} AlarmRegDef;

typedef enum {
  CIE = 0,
  AIE,
  SIE,
  TIME_12_24,
  SR,
  STOP,
  T,
  CAP_SEL,
} Control1;

typedef enum {
  CTBIE = 0,
  CTAIE,
  WTAIE,
  AF,
  SF,
  CTBF,
  CTAF,
  WTAF,
} Control2;

typedef enum {
  CTBIE = 0,
  CTAIE,
  WTAIE,
  AF,
  SF,
  CTBF,
  CTAF,
  WTAF,
} Control2;

typedef enum {
  BLIE = 0,
  BSIE,
  BLF,
  BSF,
  PM,
} Control3;
