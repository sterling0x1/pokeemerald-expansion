#ifndef GUARD_RANDOMIZER_H
#define GUARD_RANDOMIZER_H

#include "global.h"

enum Species Randomizer_GetWildSpecies(enum Species species);
enum Species Randomizer_GetStarterSpecies(enum Species originalSpecies, u8 starterId);
void Randomizer_InitNewGameSeed(void);
void Randomizer_RerollSeed(void);
u32 Randomizer_GetSeed(void);
bool32 Randomizer_IsWildEnabled(void);
bool32 Randomizer_IsStarterEnabled(void);
void Randomizer_SetWildEnabled(bool32 enabled);
void Randomizer_SetStarterEnabled(bool32 enabled);

#endif // GUARD_RANDOMIZER_H
