#include "global.h"
#include "module_manager.h"
#include "config/modules.h"

#if MODULE_POKEMON_RULES_ENABLED

static const u8 sModuleName[] = _("POKéMON RULES");

const struct ModuleDescriptor gPokemonRulesModuleDescriptor =
{
    .id = MODULE_ID_POKEMON_RULES,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
};

#endif // MODULE_POKEMON_RULES_ENABLED
