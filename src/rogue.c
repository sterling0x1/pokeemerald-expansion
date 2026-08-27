#include "global.h"
#include "game_mode.h"
#include "module_save.h"
#include "random.h"
#include "rogue.h"
#include "config/module_ids.h"
#include "config/modules.h"

#if MODULE_ROGUE_ENABLED

#define ROGUE_SAVE_MAGIC 0x524F4731 // ROG1

STATIC_ASSERT(sizeof(struct RogueSaveData) == 32, RogueSaveDataSize);

static u32 Hash32(u32 value)
{
    value ^= value >> 16;
    value *= 0x7FEB352D;
    value ^= value >> 15;
    value *= 0x846CA68B;
    return value ^ (value >> 16);
}

static u32 GetTrainerSeed(void)
{
    u32 trainerId;

    memcpy(&trainerId, gSaveBlock2Ptr->playerTrainerId, sizeof(trainerId));
    return Hash32(trainerId ^ 0x524F4755);
}

static struct RogueSaveData *GetSaveData(void)
{
    u16 version;
    u16 size;
    struct RogueSaveData *data = ModuleSave_GetChunk(MODULE_ID_ROGUE, &version, &size);

    if (data == NULL || version != ROGUE_SAVE_VERSION || size != sizeof(*data) || data->magic != ROGUE_SAVE_MAGIC)
        return NULL;
    return data;
}

static struct RogueSaveData *CreateSaveData(u32 seed, enum RogueRunState state)
{
    struct RogueSaveData *data = ModuleSave_RecreateChunk(MODULE_ID_ROGUE, ROGUE_SAVE_VERSION, sizeof(*data));

    if (data == NULL)
        return NULL;
    data->magic = ROGUE_SAVE_MAGIC;
    data->baseSeed = seed != 0 ? seed : 1;
    data->runSeed = Hash32(data->baseSeed ^ 0x52554E31);
    data->state = state;
    return data;
}

void Rogue_InitNewSave(void)
{
    if (GameMode_GetPending() == GAME_MODE_ROGUE)
        CreateSaveData(Random32(), ROGUE_RUN_READY);
}

void Rogue_LoadSave(void)
{
    if (Rogue_IsModeActive() && GetSaveData() == NULL)
        CreateSaveData(GetTrainerSeed(), ROGUE_RUN_READY);
}

bool32 Rogue_IsModeActive(void)
{
    return GameMode_GetActive() == GAME_MODE_ROGUE;
}

bool32 Rogue_IsRunActive(void)
{
    return Rogue_GetRunState() == ROGUE_RUN_ACTIVE;
}

enum RogueRunState Rogue_GetRunState(void)
{
    struct RogueSaveData *data = GetSaveData();

    return data != NULL ? data->state : ROGUE_RUN_INACTIVE;
}

void Rogue_BeginRun(u32 seed)
{
    struct RogueSaveData *data = GetSaveData();

    if (!Rogue_IsModeActive() || data == NULL)
        return;
    data->runNumber++;
    data->runSeed = seed != 0 ? seed : Hash32(data->baseSeed ^ data->runNumber ^ 0x52554E31);
    data->rngCounter = 0;
    data->state = ROGUE_RUN_ACTIVE;
    data->phase = 0;
    data->currentRoom = 0;
    data->roomCount = 0;
}

void Rogue_EndRun(bool32 victory)
{
    struct RogueSaveData *data = GetSaveData();

    if (Rogue_IsModeActive() && data != NULL)
        data->state = victory ? ROGUE_RUN_COMPLETE : ROGUE_RUN_FAILED;
}

u32 Rogue_GetBaseSeed(void)
{
    struct RogueSaveData *data = GetSaveData();

    return data != NULL ? data->baseSeed : 0;
}

u32 Rogue_GetRunSeed(void)
{
    struct RogueSaveData *data = GetSaveData();

    return data != NULL ? data->runSeed : 0;
}

u32 Rogue_Random(enum RogueRandomScope scope, u32 key)
{
    return Hash32(Rogue_GetRunSeed() ^ (0x9E3779B9 * (scope + 1)) ^ key);
}

u32 Rogue_RandomRange(enum RogueRandomScope scope, u32 key, u32 range)
{
    if (range == 0)
        return 0;
    return Rogue_Random(scope, key) % range;
}

#endif // MODULE_ROGUE_ENABLED
