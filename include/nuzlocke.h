#ifndef GUARD_NUZLOCKE_H
#define GUARD_NUZLOCKE_H

#include "global.h"
#include "config/modules.h"

#define NUZLOCKE_NO_TYPE 0xFF

struct Pokemon;
struct BoxPokemon;

#if MODULE_NUZLOCKE_ENABLED
void Nuzlocke_InitRunData(void);
void Nuzlocke_EnsureRunData(void);
bool32 Nuzlocke_IsActive(void);
bool32 Nuzlocke_IsLocationUsed(bool32 isStaticEncounter, u8 mapSection);
void Nuzlocke_SetLocationUsed(bool32 isStaticEncounter, u8 mapSection);
u8 Nuzlocke_GetMonotype(void);
void Nuzlocke_SetMonotype(u8 type);
bool32 Nuzlocke_AreSettingsLocked(void);
void Nuzlocke_LockSettings(void);
u8 Nuzlocke_GetBattleStyle(void);
bool32 Nuzlocke_HasRunEnded(void);
void Nuzlocke_EndRun(void);
void Nuzlocke_BeginWildEncounter(void);
void Nuzlocke_SetNextEncounterStatic(void);
bool32 Nuzlocke_CanCatchCurrentEncounter(enum BattlerId battler);
void Nuzlocke_CommitCaughtMon(enum BattlerId battler);
bool32 Nuzlocke_IsSpeciesAllowed(enum Species species);
bool32 Nuzlocke_IsMonUsable(struct Pokemon *mon);
bool32 Nuzlocke_IsBoxMonUsable(struct BoxPokemon *boxMon);
void Nuzlocke_ProcessFaintedParty(void);
void Nuzlocke_ProcessFieldFaints(void);
void Nuzlocke_PrepareGiftMon(struct Pokemon *mon);
void Nuzlocke_CommitGiftMon(bool32 wasGiven);
bool32 Nuzlocke_TryRestoreUsablePartyFromPC(void);
#else
static inline void Nuzlocke_InitRunData(void) {}
static inline void Nuzlocke_EnsureRunData(void) {}
static inline bool32 Nuzlocke_IsActive(void) { return FALSE; }
static inline bool32 Nuzlocke_IsLocationUsed(bool32 isStaticEncounter, u8 mapSection) { return FALSE; }
static inline void Nuzlocke_SetLocationUsed(bool32 isStaticEncounter, u8 mapSection) {}
static inline u8 Nuzlocke_GetMonotype(void) { return NUZLOCKE_NO_TYPE; }
static inline void Nuzlocke_SetMonotype(u8 type) {}
static inline bool32 Nuzlocke_AreSettingsLocked(void) { return FALSE; }
static inline void Nuzlocke_LockSettings(void) {}
static inline u8 Nuzlocke_GetBattleStyle(void) { return OPTIONS_BATTLE_STYLE_SHIFT; }
static inline bool32 Nuzlocke_HasRunEnded(void) { return FALSE; }
static inline void Nuzlocke_EndRun(void) {}
static inline void Nuzlocke_BeginWildEncounter(void) {}
static inline void Nuzlocke_SetNextEncounterStatic(void) {}
static inline bool32 Nuzlocke_CanCatchCurrentEncounter(enum BattlerId battler) { return TRUE; }
static inline void Nuzlocke_CommitCaughtMon(enum BattlerId battler) {}
static inline bool32 Nuzlocke_IsSpeciesAllowed(enum Species species) { return TRUE; }
static inline bool32 Nuzlocke_IsMonUsable(struct Pokemon *mon) { return TRUE; }
static inline bool32 Nuzlocke_IsBoxMonUsable(struct BoxPokemon *boxMon) { return TRUE; }
static inline void Nuzlocke_ProcessFaintedParty(void) {}
static inline void Nuzlocke_ProcessFieldFaints(void) {}
static inline void Nuzlocke_PrepareGiftMon(struct Pokemon *mon) {}
static inline void Nuzlocke_CommitGiftMon(bool32 wasGiven) {}
static inline bool32 Nuzlocke_TryRestoreUsablePartyFromPC(void) { return FALSE; }
#endif

#endif // GUARD_NUZLOCKE_H
