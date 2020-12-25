#pragma once

typedef enum {
  CR1 = 0,
  CR2,
  CR3,
  SECONDS,
  MINUTES,
  HOURS,
  DAYS,
  WEEKDAYS,
  MONTHS,
  YEARS,
  MINUTE_ALARM,
  HOUR_ALARM,
  DAY_ALARM,
  WEEKDAY_ALARM,
  OFFSET,
} Pcf8523RegDef;

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
