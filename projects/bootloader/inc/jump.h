#pragma once

#include <stdint.h>
#include <stdnoreturn.h>

// Reset main stack pointer to main stack pointer, and jump to program counter
noreturn __attribute__((naked)) void perform_jump(uint32_t sp, uint32_t pc);
