#include "global.h"
#include "module_manager.h"
#include "module_registry.h"
#include "module_save.h"

static u32 ModuleManager_GetEnabledMask(void);

bool32 ModuleManager_IsEnabled(enum ModuleId id)
{
    return id < MODULE_ID_COUNT && gModuleRegistry[id] != NULL;
}

const struct ModuleDescriptor *ModuleManager_GetDescriptor(enum ModuleId id)
{
    if (!ModuleManager_IsEnabled(id))
        return NULL;
    return gModuleRegistry[id];
}

bool32 ModuleManager_IsRegistryValid(void)
{
    u32 i;
    u32 chunkCount = 0;
    u32 chunkBytes = 0;

    for (i = 0; i < MODULE_ID_COUNT; i++)
    {
        const struct ModuleDescriptor *module;

        if (!ModuleManager_IsEnabled(i))
            continue;
        module = gModuleRegistry[i];
        if (module->id != i
         || module->requiredEngineApiVersion != MODULE_ENGINE_API_VERSION
         || ((module->saveVersion == 0) != (module->saveSize == 0))
         || (module->requiredModules & ~((1u << MODULE_ID_COUNT) - 1)) != 0
         || (module->requiredModules & ~ModuleManager_GetEnabledMask()) != 0)
            return FALSE;
        if (module->saveSize != 0)
        {
            chunkCount++;
            chunkBytes += module->saveSize;
        }
    }
    return chunkCount <= MODULE_SAVE_MAX_CHUNKS && chunkBytes <= MODULE_SAVE_DATA_CAPACITY;
}

static u32 ModuleManager_GetEnabledMask(void)
{
    u32 i;
    u32 mask = 0;

    for (i = 0; i < MODULE_ID_COUNT; i++)
    {
        if (ModuleManager_IsEnabled(i))
            mask |= MODULE_MASK(i);
    }
    return mask;
}

static void RunLifecycleCallbacks(bool32 newSave)
{
    u32 i;

    for (i = 0; i < MODULE_ID_COUNT; i++)
    {
        const struct ModuleDescriptor *module = ModuleManager_GetDescriptor(i);
        ModuleLifecycleCallback callback;

        if (module == NULL)
            continue;
        callback = newSave ? module->initNewSave : module->loadSave;
        if (callback != NULL)
            callback();
    }
}

void ModuleManager_InitNewSave(void)
{
    if (!ModuleManager_IsRegistryValid())
    {
        AGB_ASSERT(FALSE);
        return;
    }
    ModuleSave_InitNewSave();
    RunLifecycleCallbacks(TRUE);
}

void ModuleManager_LoadSave(void)
{
    if (!ModuleManager_IsRegistryValid())
    {
        AGB_ASSERT(FALSE);
        return;
    }
    ModuleSave_LoadSave();
    RunLifecycleCallbacks(FALSE);
}
