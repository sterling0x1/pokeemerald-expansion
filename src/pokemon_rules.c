#include "global.h"
#include "pokemon_rules.h"

#if MODULE_POKEMON_RULES_ENABLED

#include "extended_options.h"

bool32 PokemonRules_IsExpOnCatchEnabled(void)
{
    return ExtendedOptions_Get(EXT_OPT_EXP_ON_CATCH);
}

bool32 PokemonRules_IsPartyExpShareEnabled(void)
{
    return ExtendedOptions_Get(EXT_OPT_PARTY_EXP_SHARE);
}

u8 PokemonRules_GetShinyOdds(void)
{
    return ExtendedOptions_Get(EXT_OPT_SHINY_ODDS);
}

#endif // MODULE_POKEMON_RULES_ENABLED
