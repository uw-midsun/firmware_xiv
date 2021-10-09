#pragma once

#include <stdint.h>
#include <stdnoreturn.h>

// Reset main stack pointer to main stack pointer, and jump to program counter
static noreturn __attribute__((naked)) void prv_perform_jump(uint32_t sp, uint32_t pc);
