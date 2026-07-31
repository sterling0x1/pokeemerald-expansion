#include "global.h"
#include "module_manager.h"
#include "config/modules.h"

#if MODULE_BATTLE_PACING_ENABLED

static const u8 sModuleName[] = _("BATTLE PACING");

const struct ModuleDescriptor gBattlePacingModuleDescriptor =
{
    .id = MODULE_ID_BATTLE_PACING,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
};

#endif // MODULE_BATTLE_PACING_ENABLED
