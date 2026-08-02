#ifndef GUARD_OVERWORLD_FEATURES_H
#define GUARD_OVERWORLD_FEATURES_H

#include "global.h"
#include "config/modules.h"

#if MODULE_OVERWORLD_FEATURES_ENABLED
bool32 OverworldFeatures_UseVisibleEncounters(void);
bool32 OverworldFeatures_AreFollowersEnabled(void);
#else
static inline bool32 OverworldFeatures_UseVisibleEncounters(void) { return FALSE; }
static inline bool32 OverworldFeatures_AreFollowersEnabled(void) { return FALSE; }
#endif

#endif // GUARD_OVERWORLD_FEATURES_H
