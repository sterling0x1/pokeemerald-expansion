#include "global.h"
#include "module_manager.h"
#include "config/modules.h"

#if MODULE_DIFFICULTY_ENABLED

static const u8 sModuleName[] = _("DIFFICULTY");

const struct ModuleDescriptor gDifficultyModuleDescriptor =
{
    .id = MODULE_ID_DIFFICULTY,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
};

#endif // MODULE_DIFFICULTY_ENABLED
