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
#define MODULE_GAME_MODES_ENABLED TRUE
#define MODULE_ACTIVE_BATTLE_ENABLED TRUE
#define MODULE_KEY_ITEM_DOCK_ENABLED TRUE
#define MODULE_MODERN_SHOP_UI_ENABLED TRUE
#define MODULE_GAME_CORNER_ENABLED TRUE

// Legacy compatibility name used by the imported Modern Shop UI. This must
// live here rather than general.h: global.h reads general.h before TRUE/FALSE
// are defined by the GBA headers.
#if MODULE_MODERN_SHOP_UI_ENABLED
#define MUDSKIP_SHOP_UI
#endif
// Applies a 50% final-damage penalty to successful Bench Attacker moves.
// Set FALSE to keep Bench Attacker available at full damage.
#define MODULE_BENCH_ATTACKER_HALF_DAMAGE_ENABLED TRUE

// Claims special flash sector 31 when enabled. Recorded Battle persistence is
// unavailable, but the normal two save slots are not resized or reinterpreted.
#define MODULE_SHARED_TRANSFER_BOX_ENABLED TRUE

#endif // GUARD_CONFIG_MODULES_H
