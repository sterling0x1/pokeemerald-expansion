#include "global.h"
#include "module_manager.h"
#include "config/modules.h"

#if MODULE_BENCH_ATTACKER_HALF_DAMAGE_ENABLED

static const u8 sModuleName[] = _("BENCH DAMAGE RULE");

const struct ModuleDescriptor gBenchAttackerDamageModuleDescriptor =
{
    .id = MODULE_ID_BENCH_ATTACKER_DAMAGE,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
};

#endif
