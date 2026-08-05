#include "global.h"
#include "cheats.h"
#include "extended_options.h"

#if MODULE_CHEATS_ENABLED
#define CHEAT_SAVE_MAGIC 0x43485431

void Cheats_Init(void)
{
    memset(&gSaveBlock3Ptr->cheats, 0, sizeof(gSaveBlock3Ptr->cheats));
    gSaveBlock3Ptr->cheats.magic = CHEAT_SAVE_MAGIC;
    gSaveBlock3Ptr->cheats.evGain = CHEAT_EV_1X;
}

void Cheats_EnsureInitialized(void)
{
    if (gSaveBlock3Ptr->cheats.magic != CHEAT_SAVE_MAGIC)
        Cheats_Init();
}

u8 Cheats_Get(enum CheatOption option)
{
    Cheats_EnsureInitialized();
    switch (option)
    {
    case CHEAT_MONEY_MULTIPLIER: return gSaveBlock3Ptr->cheats.moneyMultiplier;
    case CHEAT_EXP_MULTIPLIER: return ExtendedOptions_Get(EXT_OPT_EXP_MULTIPLIER);
    case CHEAT_CATCH_RATE: return gSaveBlock3Ptr->cheats.catchRate;
    case CHEAT_HATCH_SPEED: return gSaveBlock3Ptr->cheats.hatchSpeed;
    case CHEAT_EV_GAIN: return gSaveBlock3Ptr->cheats.evGain;
    case CHEAT_MART_PRICES: return gSaveBlock3Ptr->cheats.martPrices;
    case CHEAT_INFINITE_REPEL: return gSaveBlock3Ptr->cheats.infiniteRepel;
    default: return 0;
    }
}

u8 Cheats_GetMax(enum CheatOption option)
{
    return option == CHEAT_MART_PRICES ? CHEAT_MART_FREE
         : option == CHEAT_INFINITE_REPEL ? 1 : 3;
}

void Cheats_Set(enum CheatOption option, u8 value)
{
    Cheats_EnsureInitialized();
    if (value > Cheats_GetMax(option))
        value = Cheats_GetMax(option);
    switch (option)
    {
    case CHEAT_MONEY_MULTIPLIER: gSaveBlock3Ptr->cheats.moneyMultiplier = value; break;
    case CHEAT_EXP_MULTIPLIER: ExtendedOptions_Set(EXT_OPT_EXP_MULTIPLIER, value); break;
    case CHEAT_CATCH_RATE: gSaveBlock3Ptr->cheats.catchRate = value; break;
    case CHEAT_HATCH_SPEED: gSaveBlock3Ptr->cheats.hatchSpeed = value; break;
    case CHEAT_EV_GAIN: gSaveBlock3Ptr->cheats.evGain = value; break;
    case CHEAT_MART_PRICES: gSaveBlock3Ptr->cheats.martPrices = value; break;
    case CHEAT_INFINITE_REPEL: gSaveBlock3Ptr->cheats.infiniteRepel = value; break;
    default: return;
    }
    if ((option == CHEAT_EV_GAIN && value != CHEAT_EV_1X) || (option != CHEAT_EV_GAIN && value != 0))
        gSaveBlock3Ptr->cheats.cheatsUsed = TRUE;
}

void Cheats_ResetDefaults(void)
{
    bool32 used;
    Cheats_EnsureInitialized();
    used = gSaveBlock3Ptr->cheats.cheatsUsed;
    Cheats_Init();
    ExtendedOptions_Set(EXT_OPT_EXP_MULTIPLIER, EXP_MULTIPLIER_1X);
    gSaveBlock3Ptr->cheats.cheatsUsed = used;
}

bool32 Cheats_WereUsed(void)
{
    Cheats_EnsureInitialized();
    return gSaveBlock3Ptr->cheats.cheatsUsed;
}

u32 Cheats_ApplyMoneyMultiplier(u32 amount) { return amount * (Cheats_Get(CHEAT_MONEY_MULTIPLIER) + 1); }
u32 Cheats_ApplyExpMultiplier(u32 amount) { return amount * (Cheats_Get(CHEAT_EXP_MULTIPLIER) + 1); }
u32 Cheats_ApplyCatchRate(u32 odds)
{
    switch (Cheats_Get(CHEAT_CATCH_RATE))
    {
    case CHEAT_CATCH_2X: return odds * 2;
    case CHEAT_CATCH_4X: return odds * 4;
    case CHEAT_CATCH_GUARANTEED: return 0xFFFFFFFF;
    default: return odds;
    }
}
u32 Cheats_GetHatchStepMultiplier(void)
{
    static const u8 sValues[] = {1, 2, 4, 255};
    return sValues[Cheats_Get(CHEAT_HATCH_SPEED)];
}
u32 Cheats_ApplyEVMultiplier(u32 amount)
{
    static const u8 sValues[] = {0, 1, 2, 4};
    return amount * sValues[Cheats_Get(CHEAT_EV_GAIN)];
}
u32 Cheats_ApplyMartPrice(u32 price)
{
    switch (Cheats_Get(CHEAT_MART_PRICES))
    {
    case CHEAT_MART_HALF: return max(1, price / 2);
    case CHEAT_MART_FREE: return 0;
    default: return price;
    }
}
bool32 Cheats_InfiniteRepelEnabled(void) { return Cheats_Get(CHEAT_INFINITE_REPEL); }
#endif // MODULE_CHEATS_ENABLED
