#include "global.h"
#include "game_mode.h"
#include "module_manager.h"

static const u8 sModuleName[] = _("GAME MODES");

const struct ModuleDescriptor gGameModesModuleDescriptor =
{
    .id = MODULE_ID_GAME_MODES,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
    .saveVersion = 1,
    .saveSize = 4,
    .optionalModules = MODULE_MASK(MODULE_ID_RANDOMIZER) | MODULE_MASK(MODULE_ID_NUZLOCKE),
    .initNewSave = GameMode_InitNewSave,
    .loadSave = GameMode_LoadSave,
};
