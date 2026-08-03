#ifndef GUARD_GAME_MODE_H
#define GUARD_GAME_MODE_H

#include "global.h"

enum GameMode
{
    GAME_MODE_VANILLA,
    GAME_MODE_NUZLOCKE,
    GAME_MODE_CARNAGE,
    GAME_MODE_COUNT,
};

void GameMode_InitNewSave(void);
void GameMode_LoadSave(void);
void GameMode_SetPending(enum GameMode mode);
enum GameMode GameMode_GetPending(void);
enum GameMode GameMode_GetActive(void);
const u8 *GameMode_GetName(enum GameMode mode);
u8 GameMode_RandomizeWildLevel(u8 level, u32 key);
u8 GameMode_RandomizeTrainerLevel(u8 level, u32 key);
u8 GameMode_RandomizeGiftLevel(u8 level, u32 key);

#endif // GUARD_GAME_MODE_H
