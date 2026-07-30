#ifndef GUARD_RANDOMIZER_H
#define GUARD_RANDOMIZER_H

#include "global.h"
#include "config/modules.h"

#if MODULE_RANDOMIZER_ENABLED
enum Species Randomizer_GetWildSpecies(enum Species species);
enum Species Randomizer_GetStarterSpecies(enum Species originalSpecies, u8 starterId);
enum Species Randomizer_GetTrainerSpecies(enum Species originalSpecies, u32 key);
enum Species Randomizer_GetGiftStaticSpecies(enum Species originalSpecies, u32 key);
void Randomizer_InitNewGameSeed(void);
void Randomizer_LoadSave(void);
void Randomizer_RerollSeed(void);
u32 Randomizer_GetSeed(void);
bool32 Randomizer_IsInitialized(void);
bool32 Randomizer_IsWildEnabled(void);
bool32 Randomizer_IsStarterEnabled(void);
bool32 Randomizer_IsTrainerEnabled(void);
bool32 Randomizer_IsGiftStaticEnabled(void);
bool32 Randomizer_IsAllDataEnabled(void);
void Randomizer_SetWildEnabled(bool32 enabled);
void Randomizer_SetStarterEnabled(bool32 enabled);
void Randomizer_SetTrainerEnabled(bool32 enabled);
void Randomizer_SetGiftStaticEnabled(bool32 enabled);
void Randomizer_SetAllDataEnabled(bool32 enabled);
#else
static inline enum Species Randomizer_GetWildSpecies(enum Species species) { return species; }
static inline enum Species Randomizer_GetStarterSpecies(enum Species species, u8 starterId) { return species; }
static inline enum Species Randomizer_GetTrainerSpecies(enum Species species, u32 key) { return species; }
static inline enum Species Randomizer_GetGiftStaticSpecies(enum Species species, u32 key) { return species; }
static inline void Randomizer_InitNewGameSeed(void) {}
static inline void Randomizer_LoadSave(void) {}
static inline void Randomizer_RerollSeed(void) {}
static inline u32 Randomizer_GetSeed(void) { return 0; }
static inline bool32 Randomizer_IsInitialized(void) { return FALSE; }
static inline bool32 Randomizer_IsWildEnabled(void) { return FALSE; }
static inline bool32 Randomizer_IsStarterEnabled(void) { return FALSE; }
static inline bool32 Randomizer_IsTrainerEnabled(void) { return FALSE; }
static inline bool32 Randomizer_IsGiftStaticEnabled(void) { return FALSE; }
static inline bool32 Randomizer_IsAllDataEnabled(void) { return FALSE; }
static inline void Randomizer_SetWildEnabled(bool32 enabled) {}
static inline void Randomizer_SetStarterEnabled(bool32 enabled) {}
static inline void Randomizer_SetTrainerEnabled(bool32 enabled) {}
static inline void Randomizer_SetGiftStaticEnabled(bool32 enabled) {}
static inline void Randomizer_SetAllDataEnabled(bool32 enabled) {}
#endif

#endif // GUARD_RANDOMIZER_H
