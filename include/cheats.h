#ifndef GUARD_CHEATS_H
#define GUARD_CHEATS_H

#include "global.h"

enum CheatOption
{
    CHEAT_MONEY_MULTIPLIER,
    CHEAT_EXP_MULTIPLIER,
    CHEAT_CATCH_RATE,
    CHEAT_HATCH_SPEED,
    CHEAT_EV_GAIN,
    CHEAT_MART_PRICES,
    CHEAT_INFINITE_REPEL,
    CHEAT_COUNT,
};

enum { CHEAT_MULTIPLIER_1X, CHEAT_MULTIPLIER_2X, CHEAT_MULTIPLIER_3X, CHEAT_MULTIPLIER_4X };
enum { CHEAT_CATCH_NORMAL, CHEAT_CATCH_2X, CHEAT_CATCH_4X, CHEAT_CATCH_GUARANTEED };
enum { CHEAT_HATCH_NORMAL, CHEAT_HATCH_2X, CHEAT_HATCH_4X, CHEAT_HATCH_INSTANT };
enum { CHEAT_EV_OFF, CHEAT_EV_1X, CHEAT_EV_2X, CHEAT_EV_4X };
enum { CHEAT_MART_NORMAL, CHEAT_MART_HALF, CHEAT_MART_FREE };

void Cheats_Init(void);
void Cheats_EnsureInitialized(void);
u8 Cheats_Get(enum CheatOption option);
void Cheats_Set(enum CheatOption option, u8 value);
u8 Cheats_GetMax(enum CheatOption option);
void Cheats_ResetDefaults(void);
bool32 Cheats_WereUsed(void);
u32 Cheats_ApplyMoneyMultiplier(u32 amount);
u32 Cheats_ApplyCatchRate(u32 odds);
u32 Cheats_GetHatchStepMultiplier(void);
u32 Cheats_ApplyEVMultiplier(u32 amount);
u32 Cheats_ApplyMartPrice(u32 price);
bool32 Cheats_InfiniteRepelEnabled(void);

#endif
