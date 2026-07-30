#include "global.h"
#include "module_manager.h"
#include "randomizer.h"
#include "config/modules.h"

#if MODULE_RANDOMIZER_ENABLED

static const u8 sModuleName[] = _("RANDOMIZER");

const struct ModuleDescriptor gRandomizerModuleDescriptor =
{
    .id = MODULE_ID_RANDOMIZER,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
    .saveVersion = 1,
    .saveSize = 12,
    .optionalModules = MODULE_MASK(MODULE_ID_NUZLOCKE),
    .initNewSave = Randomizer_InitNewGameSeed,
    .loadSave = Randomizer_LoadSave,
};

#endif // MODULE_RANDOMIZER_ENABLED
