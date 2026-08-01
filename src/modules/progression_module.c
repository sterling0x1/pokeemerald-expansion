#include "global.h"
#include "module_manager.h"
#include "config/modules.h"

#if MODULE_PROGRESSION_ENABLED

static const u8 sModuleName[] = _("PROGRESSION");

const struct ModuleDescriptor gProgressionModuleDescriptor =
{
    .id = MODULE_ID_PROGRESSION,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
};

#endif // MODULE_PROGRESSION_ENABLED
