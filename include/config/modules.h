#ifndef GUARD_CONFIG_MODULES_H
#define GUARD_CONFIG_MODULES_H

// Optional gameplay modules registered with the shared module lifecycle.
// These switches currently control registration and lifecycle ownership.
// Runtime hook isolation is migrated module-by-module before a switch can
// guarantee that all code and UI belonging to that module are omitted.
#define MODULE_RANDOMIZER_ENABLED TRUE
#define MODULE_NUZLOCKE_ENABLED   TRUE
#define MODULE_CHEATS_ENABLED     TRUE
#define MODULE_QOL_ENABLED        TRUE
#define MODULE_BATTLE_PACING_ENABLED TRUE
#define MODULE_PROGRESSION_ENABLED TRUE
#define MODULE_POKEMON_RULES_ENABLED TRUE
#define MODULE_START_MENU_UI_ENABLED TRUE
#define MODULE_DIFFICULTY_ENABLED TRUE
#define MODULE_OVERWORLD_FEATURES_ENABLED TRUE

#endif // GUARD_CONFIG_MODULES_H
