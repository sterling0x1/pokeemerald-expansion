#include "global.h"
#include "module_manager.h"
#include "nuzlocke.h"
#include "config/modules.h"

#if MODULE_NUZLOCKE_ENABLED

static const u8 sModuleName[] = _("NUZLOCKE");

const struct ModuleDescriptor gNuzlockeModuleDescriptor =
{
    .id = MODULE_ID_NUZLOCKE,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
    .legacySaveSize = sizeof(struct NuzlockeSaveData),
    .initNewSave = Nuzlocke_InitRunData,
    .loadSave = Nuzlocke_EnsureRunData,
};

#endif // MODULE_NUZLOCKE_ENABLED
