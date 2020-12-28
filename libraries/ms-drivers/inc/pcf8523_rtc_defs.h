#pragma once

typedef enum {
  CR1 = 0,
  CR2,
  CR3,
  NUM_CONTROL_REG,
} Pcf8523ControlReg;

typedef enum {
  SECONDS,
  MINUTES,
  HOURS,
  DAYS,
  WEEKDAYS,
  MONTHS,
  YEARS,
  NUM_TIME_REG,
} Pcf8523TimeReg;

typedef enum {
  MINUTE_ALARM,
  HOUR_ALARM,
  DAY_ALARM,
  WEEKDAY_ALARM,
  NUM_ALARM_REG,
} Pcf8523AlarmReg;

typedef enum {
  OFFSET,
  NUM_OFFSET_REG,
} Pcf8523OffsetReg;

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
  BLIE = 0,
  BSIE,
  BLF,
  BSF,
  PM,
} Control3;
