#include "global.h"
#include "battle_pacing.h"
#include "extended_options.h"
#include "config/modules.h"

#if MODULE_BATTLE_PACING_ENABLED

u8 BattlePacing_GetSpeed(void)
{
    return ExtendedOptions_Get(EXT_OPT_BATTLE_SPEED);
}

bool32 BattlePacing_IsFastIntroEnabled(void)
{
    return ExtendedOptions_Get(EXT_OPT_FAST_BATTLE_INTRO);
}

bool32 BattlePacing_IsFastHpEnabled(void)
{
    return ExtendedOptions_Get(EXT_OPT_FAST_HP_BARS);
}

bool32 BattlePacing_IsFastExpEnabled(void)
{
    return ExtendedOptions_Get(EXT_OPT_FAST_EXP_BARS);
}

#endif // MODULE_BATTLE_PACING_ENABLED
