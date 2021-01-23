#pragma once

#include <stdnoreturn.h>

// This module's job is to clean up, then jump to the application code.

// Jump to the application. Does not return!
noreturn void jump_to_application(void);
