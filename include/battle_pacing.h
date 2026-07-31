#ifndef GUARD_BATTLE_PACING_H
#define GUARD_BATTLE_PACING_H

#include "global.h"
#include "config/modules.h"
#include "extended_options.h"

#if MODULE_BATTLE_PACING_ENABLED

u8 BattlePacing_GetSpeed(void);
bool32 BattlePacing_IsFastIntroEnabled(void);
bool32 BattlePacing_IsFastHpEnabled(void);
bool32 BattlePacing_IsFastExpEnabled(void);

#else

static inline u8 BattlePacing_GetSpeed(void) { return BATTLE_SPEED_NORMAL; }
static inline bool32 BattlePacing_IsFastIntroEnabled(void) { return FALSE; }
static inline bool32 BattlePacing_IsFastHpEnabled(void) { return FALSE; }
static inline bool32 BattlePacing_IsFastExpEnabled(void) { return FALSE; }

#endif // MODULE_BATTLE_PACING_ENABLED

#endif // GUARD_BATTLE_PACING_H
