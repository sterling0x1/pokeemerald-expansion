#ifndef GUARD_NUZLOCKE_H
#define GUARD_NUZLOCKE_H

#include "global.h"

#define NUZLOCKE_NO_TYPE 0xFF

struct Pokemon;
struct BoxPokemon;

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

#endif // GUARD_NUZLOCKE_H
