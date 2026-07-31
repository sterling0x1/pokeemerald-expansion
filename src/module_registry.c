#include "global.h"
#include "module_registry.h"
#include "config/modules.h"

#if MODULE_RANDOMIZER_ENABLED
extern const struct ModuleDescriptor gRandomizerModuleDescriptor;
#endif
#if MODULE_NUZLOCKE_ENABLED
extern const struct ModuleDescriptor gNuzlockeModuleDescriptor;
#endif
#if MODULE_CHEATS_ENABLED
extern const struct ModuleDescriptor gCheatsModuleDescriptor;
#endif
#if MODULE_QOL_ENABLED
extern const struct ModuleDescriptor gQolModuleDescriptor;
#endif
#if MODULE_BATTLE_PACING_ENABLED
extern const struct ModuleDescriptor gBattlePacingModuleDescriptor;
#endif

const struct ModuleDescriptor *const gModuleRegistry[MODULE_ID_COUNT] =
{
#if MODULE_RANDOMIZER_ENABLED
    [MODULE_ID_RANDOMIZER] = &gRandomizerModuleDescriptor,
#endif
#if MODULE_NUZLOCKE_ENABLED
    [MODULE_ID_NUZLOCKE] = &gNuzlockeModuleDescriptor,
#endif
#if MODULE_CHEATS_ENABLED
    [MODULE_ID_CHEATS] = &gCheatsModuleDescriptor,
#endif
#if MODULE_QOL_ENABLED
    [MODULE_ID_QOL] = &gQolModuleDescriptor,
#endif
#if MODULE_BATTLE_PACING_ENABLED
    [MODULE_ID_BATTLE_PACING] = &gBattlePacingModuleDescriptor,
#endif
};
