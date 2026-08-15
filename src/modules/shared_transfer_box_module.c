#include "global.h"
#include "module_manager.h"
#include "config/modules.h"

#if MODULE_SHARED_TRANSFER_BOX_ENABLED

static const u8 sModuleName[] = _("SHARED TRANSFER BOX");

const struct ModuleDescriptor gSharedTransferBoxModuleDescriptor =
{
    .id = MODULE_ID_SHARED_TRANSFER_BOX,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
};

#endif
