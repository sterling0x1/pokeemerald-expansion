#include "global.h"
#include "config/modules.h"
#include "data.h"
#include "event_data.h"
#include "extended_options.h"
#include "script.h"
#include "constants/battle.h"

enum DifficultyLevel GetCurrentDifficultyLevel(void)
{
#if MODULE_DIFFICULTY_ENABLED
    return ExtendedOptions_Get(EXT_OPT_DIFFICULTY);
#else
    return DIFFICULTY_NORMAL;
#endif
}

void SetCurrentDifficultyLevel(enum DifficultyLevel desiredDifficulty)
{
#if MODULE_DIFFICULTY_ENABLED
    if (desiredDifficulty > DIFFICULTY_MAX)
        desiredDifficulty = DIFFICULTY_MAX;

    ExtendedOptions_Set(EXT_OPT_DIFFICULTY, desiredDifficulty);
#endif
}

enum DifficultyLevel GetBattlePartnerDifficultyLevel(u16 partnerId)
{
#if MODULE_DIFFICULTY_ENABLED
    enum DifficultyLevel difficulty = GetCurrentDifficultyLevel();

    if (partnerId > TRAINER_PARTNER(PARTNER_NONE))
        partnerId -= TRAINER_PARTNER(PARTNER_NONE);

    if (difficulty == DIFFICULTY_NORMAL)
        return DIFFICULTY_NORMAL;

    if (gBattlePartners[difficulty][partnerId].party == NULL)
        return DIFFICULTY_NORMAL;

    return difficulty;
#else
    return DIFFICULTY_NORMAL;
#endif
}

enum DifficultyLevel GetTrainerDifficultyLevel(u16 trainerId)
{
#if MODULE_DIFFICULTY_ENABLED
    enum DifficultyLevel difficulty = GetCurrentDifficultyLevel();

    if (difficulty == DIFFICULTY_NORMAL)
        return DIFFICULTY_NORMAL;

    if (gTrainers[difficulty][trainerId].party == NULL)
        return DIFFICULTY_NORMAL;

    return difficulty;
#else
    return DIFFICULTY_NORMAL;
#endif
}

void Script_IncreaseDifficulty(void)
{
#if MODULE_DIFFICULTY_ENABLED
    enum DifficultyLevel currentDifficulty;

    if (!B_VAR_DIFFICULTY)
        return;

    currentDifficulty = GetCurrentDifficultyLevel();

    if (currentDifficulty++ > DIFFICULTY_MAX)
        return;

    Script_RequestEffects(SCREFF_V1);
    Script_RequestWriteVar(B_VAR_DIFFICULTY);

    SetCurrentDifficultyLevel(currentDifficulty);
#endif
}

void Script_DecreaseDifficulty(void)
{
#if MODULE_DIFFICULTY_ENABLED
    enum DifficultyLevel currentDifficulty;

    if (!B_VAR_DIFFICULTY)
        return;

    currentDifficulty = GetCurrentDifficultyLevel();

    if (!currentDifficulty)
        return;

    Script_RequestEffects(SCREFF_V1);
    Script_RequestWriteVar(B_VAR_DIFFICULTY);

    SetCurrentDifficultyLevel(--currentDifficulty);
#endif
}

void Script_GetDifficulty(void)
{
    Script_RequestEffects(SCREFF_V1);
    gSpecialVar_Result = GetCurrentDifficultyLevel();
}

void Script_SetDifficulty(struct ScriptContext *ctx)
{
#if MODULE_DIFFICULTY_ENABLED
    enum DifficultyLevel desiredDifficulty = ScriptReadByte(ctx);

    Script_RequestEffects(SCREFF_V1);
    Script_RequestWriteVar(B_VAR_DIFFICULTY);

    SetCurrentDifficultyLevel(desiredDifficulty);
#else
    (void)ScriptReadByte(ctx);
#endif
}
