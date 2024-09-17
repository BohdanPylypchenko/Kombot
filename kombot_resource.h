#pragma once

#include "kombot_reftypes.h"

/*
#define KOMBOT_RESOURCE_MAX_COUNT 8

void kombot_resource_add(
    KOMBOT_PTR(void) resource,
    KOMBOT_FUNC_PTR(destructor, void, KOMBOT_PTR(void))
);
*/

void kombot_resource_init(void);

void kombot_resource_freeall(void);
