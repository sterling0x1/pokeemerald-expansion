#include "global.h"
#include "module_manager.h"
#include "config/modules.h"

#if MODULE_MODERN_SHOP_UI_ENABLED

static const u8 sModuleName[] = _("MODERN SHOP UI");

const struct ModuleDescriptor gModernShopUiModuleDescriptor =
{
    .id = MODULE_ID_MODERN_SHOP_UI,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
};

#endif
