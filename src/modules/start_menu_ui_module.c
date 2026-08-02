#include "global.h"
#include "module_manager.h"
#include "config/modules.h"

#if MODULE_START_MENU_UI_ENABLED

static const u8 sModuleName[] = _("START MENU UI");

const struct ModuleDescriptor gStartMenuUiModuleDescriptor =
{
    .id = MODULE_ID_START_MENU_UI,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
};

#endif // MODULE_START_MENU_UI_ENABLED
