#ifndef GUARD_POKEMON_RULES_H
#define GUARD_POKEMON_RULES_H

#include "global.h"
#include "config/modules.h"

#if MODULE_POKEMON_RULES_ENABLED
bool32 PokemonRules_IsExpOnCatchEnabled(void);
bool32 PokemonRules_IsPartyExpShareEnabled(void);
u8 PokemonRules_GetShinyOdds(void);
#else
#include "config_changes.h"
#include "extended_options.h"
static inline bool32 PokemonRules_IsExpOnCatchEnabled(void) { return GetConfig(B_EXP_CATCH) >= GEN_6; }
static inline bool32 PokemonRules_IsPartyExpShareEnabled(void) { return FALSE; }
static inline u8 PokemonRules_GetShinyOdds(void) { return SHINY_ODDS_8192; }
#endif

#endif // GUARD_POKEMON_RULES_H
