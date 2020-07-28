#pragma once

#include <stdbool.h>
#include <stdint.h>

#define PROJ_BUF_SIZE 64
#define INVALID_PROJ_ID PROJ_BUF_SIZE

typedef uint16_t ProjectId;

bool pm_start(const char *project_name, ProjectId *proj_id);

bool pm_stop(ProjectId proj_id);

bool pm_init();
