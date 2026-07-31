#include "global.h"
#include "module_manager.h"
#include "qol.h"
#include "config/modules.h"

#if MODULE_QOL_ENABLED

static const u8 sModuleName[] = _("QUALITY OF LIFE");

const struct ModuleDescriptor gQolModuleDescriptor =
{
    .id = MODULE_ID_QOL,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
    .initNewSave = Qol_ResetRuntimeState,
    .loadSave = Qol_ResetRuntimeState,
};

#endif // MODULE_QOL_ENABLED
