#ifndef GUARD_MODULE_MANAGER_H
#define GUARD_MODULE_MANAGER_H

#include "global.h"
#include "config/module_ids.h"

#define MODULE_ENGINE_API_VERSION 1

#define MODULE_MASK(moduleId) (1u << (moduleId))

typedef void (*ModuleLifecycleCallback)(void);

struct ModuleDescriptor
{
    enum ModuleId id;
    const u8 *name;
    u16 moduleVersion;
    u16 requiredEngineApiVersion;
    u16 saveVersion;
    u16 saveSize;
    u32 requiredModules;
    u32 optionalModules;
    ModuleLifecycleCallback initNewSave;
    ModuleLifecycleCallback loadSave;
};

bool32 ModuleManager_IsEnabled(enum ModuleId id);
bool32 ModuleManager_IsRegistryValid(void);
const struct ModuleDescriptor *ModuleManager_GetDescriptor(enum ModuleId id);
void ModuleManager_InitNewSave(void);
void ModuleManager_LoadSave(void);

#endif // GUARD_MODULE_MANAGER_H
