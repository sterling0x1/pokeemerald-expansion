#include "global.h"
#include "progression.h"

#if MODULE_PROGRESSION_ENABLED

#include "extended_options.h"

u8 Progression_GetLevelCapsMode(void)
{
    return ExtendedOptions_Get(EXT_OPT_LEVEL_CAPS);
}

void Progression_SetLevelCapsMode(u8 mode)
{
    ExtendedOptions_Set(EXT_OPT_LEVEL_CAPS, mode);
}

bool32 Progression_AreLevelCapsEnabled(void)
{
    return Progression_GetLevelCapsMode() != LEVEL_CAPS_OFF;
}

bool32 Progression_IsHardLevelCapEnabled(void)
{
    return Progression_GetLevelCapsMode() == LEVEL_CAPS_HARD;
}

#endif // MODULE_PROGRESSION_ENABLED
