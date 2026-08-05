#ifndef GUARD_ACTIVE_BATTLE_H
#define GUARD_ACTIVE_BATTLE_H

#include "config/modules.h"

enum ActiveBattleTimingDifficulty
{
    ACTIVE_BATTLE_TIMING_STANDARD,
    ACTIVE_BATTLE_TIMING_HARD,
    ACTIVE_BATTLE_TIMING_EXPERT,
    ACTIVE_BATTLE_TIMING_COUNT,
};

enum ActiveBattleCriticalResult
{
    ACTIVE_BATTLE_CRIT_NONE,
    ACTIVE_BATTLE_CRIT_GOOD,
    ACTIVE_BATTLE_CRIT_PERFECT,
};

enum ActiveBattleDodgeResult
{
    ACTIVE_BATTLE_DODGE_NONE,
    ACTIVE_BATTLE_DODGE_GOOD,
    ACTIVE_BATTLE_DODGE_PERFECT,
};

struct ActiveBattleSaveData
{
    u8 criticals;
    u8 dodging;
    u8 timingDifficulty;
    u8 reserved;
};

#if MODULE_ACTIVE_BATTLE_ENABLED
void ActiveBattle_InitNewSave(void);
void ActiveBattle_LoadSave(void);
bool32 ActiveBattle_AreCriticalsEnabled(void);
void ActiveBattle_SetCriticalsEnabled(bool32 enabled);
bool32 ActiveBattle_IsDodgingEnabled(void);
void ActiveBattle_SetDodgingEnabled(bool32 enabled);
enum ActiveBattleTimingDifficulty ActiveBattle_GetTimingDifficulty(void);
void ActiveBattle_SetTimingDifficulty(enum ActiveBattleTimingDifficulty difficulty);
bool32 ActiveBattle_UpdateDamagePrompt(enum BattlerId attacker, enum BattlerId target, enum Move move);
enum ActiveBattleCriticalResult ActiveBattle_GetCriticalResult(void);
s32 ActiveBattle_AdjustDamageForDodge(s32 damage);
void ActiveBattle_FinishDamageCalc(void);
#else
static inline void ActiveBattle_InitNewSave(void) {}
static inline void ActiveBattle_LoadSave(void) {}
static inline bool32 ActiveBattle_AreCriticalsEnabled(void) { return FALSE; }
static inline void ActiveBattle_SetCriticalsEnabled(bool32 enabled) {}
static inline bool32 ActiveBattle_IsDodgingEnabled(void) { return FALSE; }
static inline void ActiveBattle_SetDodgingEnabled(bool32 enabled) {}
static inline enum ActiveBattleTimingDifficulty ActiveBattle_GetTimingDifficulty(void) { return ACTIVE_BATTLE_TIMING_STANDARD; }
static inline void ActiveBattle_SetTimingDifficulty(enum ActiveBattleTimingDifficulty difficulty) {}
static inline bool32 ActiveBattle_UpdateDamagePrompt(enum BattlerId attacker, enum BattlerId target, enum Move move) { return TRUE; }
static inline enum ActiveBattleCriticalResult ActiveBattle_GetCriticalResult(void) { return ACTIVE_BATTLE_CRIT_NONE; }
static inline s32 ActiveBattle_AdjustDamageForDodge(s32 damage) { return damage; }
static inline void ActiveBattle_FinishDamageCalc(void) {}
#endif

#endif // GUARD_ACTIVE_BATTLE_H
