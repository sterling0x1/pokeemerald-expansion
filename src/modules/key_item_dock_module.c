#include "global.h"
#include "module_manager.h"
#include "config/modules.h"

#if MODULE_KEY_ITEM_DOCK_ENABLED

static const u8 sModuleName[] = _("KEY ITEM DOCK");

const struct ModuleDescriptor gKeyItemDockModuleDescriptor =
{
    .id = MODULE_ID_KEY_ITEM_DOCK,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
};

#endif
