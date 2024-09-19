#include "pch.h"

#include "kombot_resource.h"

#include "kombot_reftypes.h"
#include "kombot_input.h"
#include "kombot_aim.h"

#define KOMBOT_RESOURCE_MAX_COUNT 8

typedef struct {
    KOMBOT_PTR(void) resource;
    KOMBOT_FUNC_PTR(destructor, void, KOMBOT_PTR(void));
} kombot_resource_atom;

typedef struct {
    kombot_resource_atom atom_arr[KOMBOT_RESOURCE_MAX_COUNT];
    size_t count;
} kombot_resource_holder;

static kombot_resource_holder resource_holder;

static void kombot_resource_add(
    KOMBOT_PTR(void) resource,
    KOMBOT_FUNC_PTR(destructor, void, KOMBOT_PTR(void))
) {
    resource_holder.atom_arr[resource_holder.count].resource = resource;
    resource_holder.atom_arr[resource_holder.count].destructor = destructor;
    resource_holder.count++;
}

void kombot_resource_init(void) {
    resource_holder.count = 0;

    kombot_input_init();
    kombot_resource_add(NULL, kombot_input_free);

    kombot_aim_init();
    kombot_resource_add(NULL, kombot_aim_free);
}

void kombot_resource_freeall(void) {
    for (size_t i = 0; i < resource_holder.count; i++) {
        KOMBOT_PTR(kombot_resource_atom) current_atom = &resource_holder.atom_arr[i];
        current_atom->destructor(current_atom->resource);
    }
}