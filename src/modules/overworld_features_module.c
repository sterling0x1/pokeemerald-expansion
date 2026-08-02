#include "global.h"
#include "module_manager.h"
#include "config/modules.h"

#if MODULE_OVERWORLD_FEATURES_ENABLED

static const u8 sModuleName[] = _("OVERWORLD FEATURES");

const struct ModuleDescriptor gOverworldFeaturesModuleDescriptor =
{
    .id = MODULE_ID_OVERWORLD_FEATURES,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
};

#endif // MODULE_OVERWORLD_FEATURES_ENABLED
