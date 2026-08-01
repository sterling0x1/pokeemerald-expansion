#ifndef GUARD_PROGRESSION_H
#define GUARD_PROGRESSION_H

#include "global.h"
#include "config/modules.h"

#if MODULE_PROGRESSION_ENABLED
u8 Progression_GetLevelCapsMode(void);
void Progression_SetLevelCapsMode(u8 mode);
bool32 Progression_AreLevelCapsEnabled(void);
bool32 Progression_IsHardLevelCapEnabled(void);
#else
static inline u8 Progression_GetLevelCapsMode(void) { return 0; }
static inline void Progression_SetLevelCapsMode(u8 mode) {}
static inline bool32 Progression_AreLevelCapsEnabled(void) { return FALSE; }
static inline bool32 Progression_IsHardLevelCapEnabled(void) { return FALSE; }
#endif

#endif // GUARD_PROGRESSION_H
