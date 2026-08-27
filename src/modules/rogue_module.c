#include "global.h"
#include "module_manager.h"
#include "rogue.h"
#include "config/modules.h"

#if MODULE_ROGUE_ENABLED

static const u8 sModuleName[] = _("ROGUE");

const struct ModuleDescriptor gRogueModuleDescriptor =
{
    .id = MODULE_ID_ROGUE,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
    .saveVersion = ROGUE_SAVE_VERSION,
    .saveSize = sizeof(struct RogueSaveData),
    .requiredModules = MODULE_MASK(MODULE_ID_GAME_MODES),
    .optionalModules = MODULE_MASK(MODULE_ID_RANDOMIZER) | MODULE_MASK(MODULE_ID_DIFFICULTY),
    .initNewSave = Rogue_InitNewSave,
    .loadSave = Rogue_LoadSave,
};

#endif // MODULE_ROGUE_ENABLED
