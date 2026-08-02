#include "global.h"
#include "overworld_features.h"

#if MODULE_OVERWORLD_FEATURES_ENABLED

#include "extended_options.h"

bool32 OverworldFeatures_UseVisibleEncounters(void)
{
    return ExtendedOptions_Get(EXT_OPT_ENCOUNTER_STYLE) == ENCOUNTER_STYLE_VISIBLE;
}

bool32 OverworldFeatures_AreFollowersEnabled(void)
{
    return ExtendedOptions_Get(EXT_OPT_FOLLOWER);
}

#endif // MODULE_OVERWORLD_FEATURES_ENABLED
