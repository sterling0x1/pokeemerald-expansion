#include "global.h"
#include "extended_options.h"
#include "qol.h"

#if MODULE_QOL_ENABLED

static bool32 sTrainerEscapePending;
static bool32 sTrainerApproachSuppressed;
static s16 sTrainerEscapeX;
static s16 sTrainerEscapeY;
static u8 sTrainerEscapeMapGroup;
static u8 sTrainerEscapeMapNum;

bool32 Qol_IsAutoRunEnabled(void)
{
    return ExtendedOptions_Get(EXT_OPT_AUTO_RUN);
}

bool32 Qol_IsRunningIndoorsEnabled(void)
{
    return ExtendedOptions_Get(EXT_OPT_RUNNING_INDOORS);
}

u8 Qol_GetItemDescriptionsMode(void)
{
    return ExtendedOptions_Get(EXT_OPT_ITEM_DESCRIPTIONS);
}

bool32 Qol_IsRepelPromptEnabled(void)
{
    return ExtendedOptions_Get(EXT_OPT_REPEL_PROMPT);
}

bool32 Qol_AreTmsReusable(void)
{
    return ExtendedOptions_Get(EXT_OPT_REUSABLE_TMS);
}

bool32 Qol_IsFieldPoisonEnabled(void)
{
    return ExtendedOptions_Get(EXT_OPT_FIELD_POISON);
}

bool32 Qol_IsTrainerEscapeEnabled(void)
{
    return ExtendedOptions_Get(EXT_OPT_TRAINER_ESCAPE);
}

bool32 Qol_IsBoxShortcutEnabled(void)
{
    return TRUE;
}

void Qol_MarkTrainerEscape(void)
{
    sTrainerEscapePending = TRUE;
    sTrainerApproachSuppressed = TRUE;
    sTrainerEscapeX = gSaveBlock1Ptr->pos.x;
    sTrainerEscapeY = gSaveBlock1Ptr->pos.y;
    sTrainerEscapeMapGroup = gSaveBlock1Ptr->location.mapGroup;
    sTrainerEscapeMapNum = gSaveBlock1Ptr->location.mapNum;
}

bool32 Qol_ConsumeTrainerEscape(void)
{
    bool32 escaped = sTrainerEscapePending;

    sTrainerEscapePending = FALSE;
    return escaped;
}

bool32 Qol_ShouldSuppressTrainerApproach(void)
{
    s16 xDistance;
    s16 yDistance;

    if (!sTrainerApproachSuppressed)
        return FALSE;

    if (gSaveBlock1Ptr->location.mapGroup != sTrainerEscapeMapGroup
     || gSaveBlock1Ptr->location.mapNum != sTrainerEscapeMapNum)
    {
        sTrainerApproachSuppressed = FALSE;
        return FALSE;
    }

    xDistance = gSaveBlock1Ptr->pos.x - sTrainerEscapeX;
    yDistance = gSaveBlock1Ptr->pos.y - sTrainerEscapeY;
    if (xDistance < 0)
        xDistance = -xDistance;
    if (yDistance < 0)
        yDistance = -yDistance;

    if (xDistance + yDistance >= 2)
    {
        sTrainerApproachSuppressed = FALSE;
        return FALSE;
    }

    return TRUE;
}

void Qol_ResetRuntimeState(void)
{
    sTrainerEscapePending = FALSE;
    sTrainerApproachSuppressed = FALSE;
}

#endif // MODULE_QOL_ENABLED
