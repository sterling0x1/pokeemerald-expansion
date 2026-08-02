#include "global.h"
#include "module_registry.h"
#include "config/modules.h"

#if MODULE_RANDOMIZER_ENABLED
extern const struct ModuleDescriptor gRandomizerModuleDescriptor;
#endif
#if MODULE_NUZLOCKE_ENABLED
extern const struct ModuleDescriptor gNuzlockeModuleDescriptor;
#endif
#if MODULE_CHEATS_ENABLED
extern const struct ModuleDescriptor gCheatsModuleDescriptor;
#endif
#if MODULE_QOL_ENABLED
extern const struct ModuleDescriptor gQolModuleDescriptor;
#endif
#if MODULE_BATTLE_PACING_ENABLED
extern const struct ModuleDescriptor gBattlePacingModuleDescriptor;
#endif
#if MODULE_PROGRESSION_ENABLED
extern const struct ModuleDescriptor gProgressionModuleDescriptor;
#endif
#if MODULE_POKEMON_RULES_ENABLED
extern const struct ModuleDescriptor gPokemonRulesModuleDescriptor;
#endif
#if MODULE_START_MENU_UI_ENABLED
extern const struct ModuleDescriptor gStartMenuUiModuleDescriptor;
#endif
#if MODULE_DIFFICULTY_ENABLED
extern const struct ModuleDescriptor gDifficultyModuleDescriptor;
#endif
#if MODULE_OVERWORLD_FEATURES_ENABLED
extern const struct ModuleDescriptor gOverworldFeaturesModuleDescriptor;
#endif

const struct ModuleDescriptor *const gModuleRegistry[MODULE_ID_COUNT] =
{
#if MODULE_RANDOMIZER_ENABLED
    [MODULE_ID_RANDOMIZER] = &gRandomizerModuleDescriptor,
#endif
#if MODULE_NUZLOCKE_ENABLED
    [MODULE_ID_NUZLOCKE] = &gNuzlockeModuleDescriptor,
#endif
#if MODULE_CHEATS_ENABLED
    [MODULE_ID_CHEATS] = &gCheatsModuleDescriptor,
#endif
#if MODULE_QOL_ENABLED
    [MODULE_ID_QOL] = &gQolModuleDescriptor,
#endif
#if MODULE_BATTLE_PACING_ENABLED
    [MODULE_ID_BATTLE_PACING] = &gBattlePacingModuleDescriptor,
#endif
#if MODULE_PROGRESSION_ENABLED
    [MODULE_ID_PROGRESSION] = &gProgressionModuleDescriptor,
#endif
#if MODULE_POKEMON_RULES_ENABLED
    [MODULE_ID_POKEMON_RULES] = &gPokemonRulesModuleDescriptor,
#endif
#if MODULE_START_MENU_UI_ENABLED
    [MODULE_ID_START_MENU_UI] = &gStartMenuUiModuleDescriptor,
#endif
#if MODULE_DIFFICULTY_ENABLED
    [MODULE_ID_DIFFICULTY] = &gDifficultyModuleDescriptor,
#endif
#if MODULE_OVERWORLD_FEATURES_ENABLED
    [MODULE_ID_OVERWORLD_FEATURES] = &gOverworldFeaturesModuleDescriptor,
#endif
};
