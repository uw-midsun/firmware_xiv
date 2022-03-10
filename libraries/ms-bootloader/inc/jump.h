#pragma once
// This library allows client to jump to an arbitrary memory address
#include <stdint.h>
#include <stdnoreturn.h>
#include <string.h>

// Reset main stack pointer to main stack pointer, and jump to program counter
noreturn void perform_jump(uint32_t sp, uint32_t pc);
