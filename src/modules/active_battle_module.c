#include "global.h"
#include "active_battle.h"
#include "module_manager.h"

#if MODULE_ACTIVE_BATTLE_ENABLED

static const u8 sModuleName[] = _("ACTIVE BATTLE");

const struct ModuleDescriptor gActiveBattleModuleDescriptor =
{
    .id = MODULE_ID_ACTIVE_BATTLE,
    .name = sModuleName,
    .moduleVersion = 1,
    .requiredEngineApiVersion = MODULE_ENGINE_API_VERSION,
    .saveVersion = 1,
    .saveSize = sizeof(struct ActiveBattleSaveData),
    .initNewSave = ActiveBattle_InitNewSave,
    .loadSave = ActiveBattle_LoadSave,
};

#endif // MODULE_ACTIVE_BATTLE_ENABLED
