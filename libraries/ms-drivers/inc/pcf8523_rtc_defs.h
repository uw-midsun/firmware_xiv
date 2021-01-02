#pragma once

typedef enum {
  CR1 = 0,
  CR2,
  CR3,
  NUM_CONTROL_REG,
} Pcf8523ControlReg;

typedef enum {
  SECONDS = 3,
  MINUTES,
  HOURS,
  DAYS,
  WEEKDAYS,
  MONTHS,
  YEARS,
  NUM_TIME_REG = 7,
} Pcf8523TimeReg;

typedef enum {
  MINUTE_ALARM = 10,
  HOUR_ALARM,
  DAY_ALARM,
  WEEKDAY_ALARM,
  NUM_ALARM_REG = 4,
} Pcf8523AlarmReg;

typedef enum {
  OFFSET = 14,
  NUM_OFFSET_REG = 1,
} Pcf8523OffsetReg;

typedef enum {
  PCF8523_CR1_CIE = 0,
  PCF8523_CR1_AIE,
  PCF8523_CR1_SIE,
  PCF8523_CR1_TIME_12_24,
  PCF8523_CR1_SR,
  PCF8523_CR1_STOP,
  PCF8523_CR1_T,
  PCF8523_CR1_CAP_SEL,
  NUM_CR1 = 8,
} Control1;

typedef enum {
  PCF8523_CR2_CTBIE = 0,
  PCF8523_CR2_CTAIE,
  PCF8523_CR2_WTAIE,
  PCF8523_CR2_AF,
  PCF8523_CR2_SF,
  PCF8523_CR2_CTBF,
  PCF8523_CR2_CTAF,
  PCF8523_CR2_WTAF,
  NUM_CR2 = 8,
} Control2;

typedef enum {
  PCF8523_CR3_BLIE = 0,
  PCF8523_CR3_BSIE,
  PCF8523_CR3_BLF,
  PCF8523_CR3_BSF,
  PCF8523_CR3_PM,
  NUM_CR3 = 8,
} Control3;
