#include "global.h"
#include "module_manager.h"
#include "config/modules.h"

#if MODULE_GAME_CORNER_ENABLED

static const u8 sModuleName[] = _("GAME CORNER");

const struct ModuleDescriptor gGameCornerModuleDescriptor =
{
    .id = MODULE_ID_GAME_CORNER,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
};

#endif
