#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#define PROJ_BUF_SIZE 64
#define INVALID_PROJ_ID PROJ_BUF_SIZE

typedef pid_t ProjectId;

bool pm_build(const char *name, bool block);

bool pm_start(const char *name, bool block);

bool pm_stop(ProjectId proj_id);

bool pm_init();
