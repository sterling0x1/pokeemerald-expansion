#include "global.h"
#include "extended_options.h"
#include "game_mode.h"
#include "module_save.h"
#include "nuzlocke.h"
#include "nuzlocke_menu.h"
#include "randomizer.h"
#include "config/module_ids.h"
#include "config/modules.h"

static const u8 sModeNameVanilla[] = _("VANILLA");

#if MODULE_GAME_MODES_ENABLED

#define GAME_MODE_SAVE_VERSION 1

struct GameModeSaveData
{
    u8 mode;
    u8 reserved[3];
};

static EWRAM_DATA enum GameMode sPendingMode = GAME_MODE_VANILLA;

static const u8 sModeNameNuzlocke[] = _("NUZLOCKE");
static const u8 sModeNameCarnage[] = _("CARNAGE");

static struct GameModeSaveData *GetSaveData(void)
{
    u16 version;
    u16 size;
    struct GameModeSaveData *data = ModuleSave_GetChunk(MODULE_ID_GAME_MODES, &version, &size);

    if (data == NULL || version != GAME_MODE_SAVE_VERSION || size != sizeof(*data))
        return NULL;
    return data;
}

static enum GameMode InferLegacyMode(void)
{
    if (GameMode_IsAvailable(GAME_MODE_NUZLOCKE) && ExtendedOptions_Get(EXT_OPT_NUZLOCKE))
        return GAME_MODE_NUZLOCKE;
    if (GameMode_IsAvailable(GAME_MODE_CARNAGE)
     && (ExtendedOptions_Get(EXT_OPT_RANDOM_ALL_DATA)
      || (Randomizer_IsInitialized() && Randomizer_IsAllDataEnabled())))
        return GAME_MODE_CARNAGE;
    return GAME_MODE_VANILLA;
}

bool32 GameMode_IsAvailable(enum GameMode mode)
{
    switch (mode)
    {
    case GAME_MODE_VANILLA:
        return TRUE;
    case GAME_MODE_NUZLOCKE:
        return MODULE_NUZLOCKE_ENABLED;
    case GAME_MODE_CARNAGE:
        return MODULE_RANDOMIZER_ENABLED;
    default:
        return FALSE;
    }
}

void GameMode_SetPending(enum GameMode mode)
{
    sPendingMode = GameMode_IsAvailable(mode) ? mode : GAME_MODE_VANILLA;
}

enum GameMode GameMode_GetPending(void)
{
    return sPendingMode;
}

void GameMode_InitNewSave(void)
{
    struct GameModeSaveData *data = ModuleSave_RecreateChunk(MODULE_ID_GAME_MODES, GAME_MODE_SAVE_VERSION, sizeof(*data));

    if (data == NULL)
        return;
    data->mode = sPendingMode;

    ExtendedOptions_Set(EXT_OPT_NUZLOCKE, FALSE);
    Randomizer_SetAllDataEnabled(FALSE);

    if (sPendingMode == GAME_MODE_NUZLOCKE)
    {
        ExtendedOptions_Set(EXT_OPT_NUZLOCKE, TRUE);
        Nuzlocke_InitRunData();
        Nuzlocke_ApplyPreset(NUZLOCKE_PRESET_STANDARD);
    }
    else if (sPendingMode == GAME_MODE_CARNAGE)
    {
        Randomizer_SetAllDataEnabled(TRUE);
    }
}

void GameMode_LoadSave(void)
{
    struct GameModeSaveData *data = GetSaveData();

    if (data == NULL)
    {
        data = ModuleSave_RecreateChunk(MODULE_ID_GAME_MODES, GAME_MODE_SAVE_VERSION, sizeof(*data));
        if (data != NULL)
            data->mode = InferLegacyMode();
    }
    else if (data->mode >= GAME_MODE_COUNT)
    {
        data->mode = InferLegacyMode();
    }
}

enum GameMode GameMode_GetActive(void)
{
    struct GameModeSaveData *data;

    GameMode_LoadSave();
    data = GetSaveData();
    return data != NULL && GameMode_IsAvailable(data->mode) ? data->mode : InferLegacyMode();
}

const u8 *GameMode_GetName(enum GameMode mode)
{
    switch (mode)
    {
    case GAME_MODE_NUZLOCKE:
        return sModeNameNuzlocke;
    case GAME_MODE_CARNAGE:
        return sModeNameCarnage;
    case GAME_MODE_VANILLA:
    default:
        return sModeNameVanilla;
    }
}

static u32 HashLevelKey(u32 value)
{
    value ^= value >> 16;
    value *= 0x7FEB352D;
    value ^= value >> 15;
    value *= 0x846CA68B;
    return value ^ (value >> 16);
}

static u8 RandomizeLevelWithSpread(u8 level, u32 key, u32 minimumSpread, u32 spreadDivisor)
{
    u32 spread;
    u32 minimum;
    u32 maximum;

    if (GameMode_GetActive() != GAME_MODE_CARNAGE)
        return level;

    spread = max(minimumSpread, level / spreadDivisor);
    minimum = level > spread ? level - spread : MIN_LEVEL;
    maximum = min(MAX_LEVEL, level + spread);
    return minimum + (HashLevelKey(Randomizer_GetSeed() ^ key ^ level) % (maximum - minimum + 1));
}

u8 GameMode_RandomizeWildLevel(u8 level, u32 key)
{
    return RandomizeLevelWithSpread(level, key ^ 0x57494C44, 5, 2);
}

u8 GameMode_RandomizeTrainerLevel(u8 level, u32 key)
{
    return RandomizeLevelWithSpread(level, key ^ 0x54524E52, 3, 3);
}

u8 GameMode_RandomizeGiftLevel(u8 level, u32 key)
{
    s32 offset;

    if (GameMode_GetActive() != GAME_MODE_CARNAGE)
        return level;

    offset = (HashLevelKey(Randomizer_GetSeed() ^ key ^ level ^ 0x47494654) % 7) - 3;
    return min(MAX_LEVEL, max(MIN_LEVEL, level + offset));
}

#else

void GameMode_InitNewSave(void)
{
}

void GameMode_LoadSave(void)
{
}

void GameMode_SetPending(enum GameMode mode)
{
    (void)mode;
}

bool32 GameMode_IsAvailable(enum GameMode mode)
{
    return mode == GAME_MODE_VANILLA;
}

enum GameMode GameMode_GetPending(void)
{
    return GAME_MODE_VANILLA;
}

enum GameMode GameMode_GetActive(void)
{
    return GAME_MODE_VANILLA;
}

const u8 *GameMode_GetName(enum GameMode mode)
{
    (void)mode;
    return sModeNameVanilla;
}

u8 GameMode_RandomizeWildLevel(u8 level, u32 key)
{
    (void)key;
    return level;
}

u8 GameMode_RandomizeTrainerLevel(u8 level, u32 key)
{
    (void)key;
    return level;
}

u8 GameMode_RandomizeGiftLevel(u8 level, u32 key)
{
    (void)key;
    return level;
}

#endif // MODULE_GAME_MODES_ENABLED
