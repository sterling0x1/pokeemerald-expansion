#include "global.h"
#include "cheats.h"
#include "module_manager.h"
#include "config/modules.h"

#if MODULE_CHEATS_ENABLED

static const u8 sModuleName[] = _("CHEATS");

const struct ModuleDescriptor gCheatsModuleDescriptor =
{
    .id = MODULE_ID_CHEATS,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
    .initNewSave = Cheats_Init,
    .loadSave = Cheats_EnsureInitialized,
};

#endif // MODULE_CHEATS_ENABLED
