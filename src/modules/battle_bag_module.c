#include "global.h"
#include "module_manager.h"
#include "config/modules.h"

#if MODULE_BATTLE_BAG_ENABLED

static const u8 sModuleName[] = _("BATTLE BAG");

const struct ModuleDescriptor gBattleBagModuleDescriptor =
{
    .id = MODULE_ID_BATTLE_BAG,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
};

#endif // MODULE_BATTLE_BAG_ENABLED
