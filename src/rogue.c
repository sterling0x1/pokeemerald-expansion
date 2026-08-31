#include "global.h"
#include "coins.h"
#include "event_data.h"
#include "game_mode.h"
#include "item.h"
#include "module_save.h"
#include "pokemon.h"
#include "random.h"
#include "rogue.h"
#include "script_pokemon_util.h"
#include "string_util.h"
#include "config/module_ids.h"
#include "config/modules.h"
#include "constants/coins.h"

#if MODULE_ROGUE_ENABLED

#define ROGUE_SAVE_MAGIC 0x524F4731 // ROG1

STATIC_ASSERT(sizeof(struct RogueSaveData) == 36, RogueSaveDataSize);

struct RogueSaveDataV2
{
    u32 magic;
    u32 baseSeed;
    u32 runSeed;
    u32 rngCounter;
    u16 runNumber;
    u8 state;
    u8 phase;
    u8 currentRoom;
    u8 roomCount;
    u8 difficulty;
    u8 currentRoomType;
    u8 selectedRoute;
    u8 pendingChoices;
    u8 routeRevision;
    u8 reserved[5];
};

STATIC_ASSERT(sizeof(struct RogueSaveDataV2) == 32, RogueSaveDataV2Size);

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

static struct RogueSaveData *MigrateSaveData(void)
{
    u16 version;
    u16 size;
    struct RogueSaveDataV2 legacy;
    struct RogueSaveDataV2 *oldData = ModuleSave_GetChunk(MODULE_ID_ROGUE, &version, &size);
    struct RogueSaveData *data;

    if (oldData == NULL || version != 2 || size != sizeof(*oldData) || oldData->magic != ROGUE_SAVE_MAGIC)
        return NULL;

    legacy = *oldData;
    data = ModuleSave_RecreateChunk(MODULE_ID_ROGUE, ROGUE_SAVE_VERSION, sizeof(*data));
    if (data == NULL)
        return NULL;

    memcpy(data, &legacy, offsetof(struct RogueSaveDataV2, reserved));
    return data;
}

static void AddPendingChips(struct RogueSaveData *data, u16 amount)
{
    data->pendingChips = min(0xFFFF, data->pendingChips + amount);
}

static void StartRun(struct RogueSaveData *data, u32 seed)
{
    data->runNumber++;
    data->runSeed = seed != 0 ? seed : Hash32(data->baseSeed ^ data->runNumber ^ 0x52554E31);
    data->rngCounter = 0;
    data->state = ROGUE_RUN_ACTIVE;
    data->phase = 0;
    data->currentRoom = 0;
    data->roomCount = 0;
    data->difficulty = 0;
    data->currentRoomType = ROGUE_ROOM_START;
    data->selectedRoute = 0;
    data->pendingChoices = TRUE;
    data->routeRevision = 0;
}

void Rogue_InitNewSave(void)
{
    if (GameMode_GetPending() == GAME_MODE_ROGUE)
    {
        struct RogueSaveData *data = CreateSaveData(Random32(), ROGUE_RUN_READY);

        if (data != NULL)
            StartRun(data, 0);
    }
}

void Rogue_LoadSave(void)
{
    if (Rogue_IsModeActive())
    {
        struct RogueSaveData *data = GetSaveData();

        if (data == NULL)
            data = MigrateSaveData();
        if (data == NULL)
            data = CreateSaveData(GetTrainerSeed(), ROGUE_RUN_READY);
        if (data != NULL && data->state == ROGUE_RUN_READY)
            StartRun(data, 0);
    }
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
    StartRun(data, seed);
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

static u32 GetRouteChoiceKey(const struct RogueSaveData *data, u8 choiceIndex)
{
    return ((u32)data->phase << 24)
         ^ ((u32)data->currentRoom << 16)
         ^ ((u32)data->routeRevision << 8)
         ^ choiceIndex;
}

static u8 GetChoiceCount(const struct RogueSaveData *data)
{
    (void)data;
    return 1;
}

static enum RogueRoomType GetChoiceType(const struct RogueSaveData *data, u8 choiceIndex)
{
    (void)choiceIndex;
    if (data->currentRoom >= ROGUE_ROUTES_PER_CHAPTER)
        return ROGUE_ROOM_BOSS;
    return ROGUE_ROOM_ROUTE;
}

static void BuildRouteChoice(const struct RogueSaveData *data, u8 choiceIndex, struct RogueRoomChoice *choice)
{
    u32 key = GetRouteChoiceKey(data, choiceIndex);

    choice->type = GetChoiceType(data, choiceIndex);
    choice->difficulty = min(5, data->phase + Rogue_RandomRange(ROGUE_RNG_ROOM, key, 2));
    choice->routeKey = Rogue_Random(ROGUE_RNG_PATH, key) & 0xFFFF;
}

u8 Rogue_GetPhase(void)
{
    struct RogueSaveData *data = GetSaveData();

    return data != NULL ? data->phase : 0;
}

u8 Rogue_GetCurrentRoom(void)
{
    struct RogueSaveData *data = GetSaveData();

    return data != NULL ? data->currentRoom : 0;
}

u8 Rogue_GetRoomCount(void)
{
    struct RogueSaveData *data = GetSaveData();

    return data != NULL ? data->roomCount : 0;
}

u8 Rogue_GetCurrentRouteVariant(void)
{
    struct RogueSaveData *data = GetSaveData();

    if (data == NULL)
        return 0;

    return min(data->roomCount, ROGUE_ROUTE_VARIANT_COUNT - 1);
}

enum RogueRoomType Rogue_GetCurrentRoomType(void)
{
    struct RogueSaveData *data = GetSaveData();

    return data != NULL ? data->currentRoomType : ROGUE_ROOM_START;
}

bool32 Rogue_HasPendingRouteChoice(void)
{
    struct RogueSaveData *data = GetSaveData();

    return data != NULL && data->state == ROGUE_RUN_ACTIVE && data->pendingChoices;
}

u8 Rogue_GetRouteChoices(struct RogueRoomChoice *choices, u8 capacity)
{
    struct RogueSaveData *data = GetSaveData();
    u8 choiceCount;
    u8 i;

    if (data == NULL || choices == NULL || capacity == 0 || !Rogue_HasPendingRouteChoice())
        return 0;

    choiceCount = min(capacity, GetChoiceCount(data));
    for (i = 0; i < choiceCount; i++)
        BuildRouteChoice(data, i, &choices[i]);
    return choiceCount;
}

bool32 Rogue_SelectRouteChoice(u8 choiceIndex)
{
    struct RogueSaveData *data = GetSaveData();
    struct RogueRoomChoice choice;

    if (data == NULL || !Rogue_HasPendingRouteChoice() || choiceIndex >= GetChoiceCount(data))
        return FALSE;

    BuildRouteChoice(data, choiceIndex, &choice);
    data->selectedRoute = choiceIndex;
    data->currentRoomType = choice.type;
    data->difficulty = choice.difficulty;
    data->pendingChoices = FALSE;
    return TRUE;
}

bool32 Rogue_CompleteCurrentRoom(void)
{
    struct RogueSaveData *data = GetSaveData();

    if (data == NULL || data->state != ROGUE_RUN_ACTIVE || data->pendingChoices)
        return FALSE;

    if (data->currentRoomType == ROGUE_ROOM_BOSS)
    {
        AddPendingChips(data, ROGUE_CHIPS_BOSS_CLEAR);
        data->phase++;
        data->currentRoom = 0;
        if (data->phase >= ROGUE_CHAPTER_COUNT)
        {
            AddPendingChips(data, ROGUE_CHIPS_VICTORY);
            data->state = ROGUE_RUN_COMPLETE;
            data->pendingChoices = FALSE;
            return TRUE;
        }
    }
    else
    {
        AddPendingChips(data, ROGUE_CHIPS_ROUTE_CLEAR);
        data->roomCount++;
        data->currentRoom++;
    }

    data->currentRoomType = ROGUE_ROOM_TRANSITION;
    data->pendingChoices = TRUE;
    data->routeRevision++;
    return TRUE;
}

void Rogue_HandleWhiteOut(void)
{
    struct RogueSaveData *data = GetSaveData();

    if (data == NULL || data->state != ROGUE_RUN_ACTIVE)
        return;

    AddPendingChips(data, ROGUE_CHIPS_DEFEAT);
    data->state = ROGUE_RUN_FAILED;
    data->currentRoomType = ROGUE_ROOM_TRANSITION;
    data->selectedRoute = 0;
    data->pendingChoices = FALSE;
    data->routeRevision++;
}

void Rogue_RecordTrainerWin(void)
{
    struct RogueSaveData *data = GetSaveData();

    if (data != NULL && data->state == ROGUE_RUN_ACTIVE)
        AddPendingChips(data, ROGUE_CHIPS_TRAINER_WIN);
}

#endif // MODULE_ROGUE_ENABLED

u16 RogueScript_SelectFirstRoute(void)
{
    return Rogue_SelectRouteChoice(0);
}

u16 RogueScript_CompleteRoom(void)
{
    return Rogue_CompleteCurrentRoom();
}

void RogueScript_BufferNextRoomNumber(void)
{
    ConvertIntToDecimalStringN(gStringVar1, Rogue_GetRoomCount() + 1, STR_CONV_MODE_LEFT_ALIGN, 3);
}

void RogueScript_BufferChapterNumber(void)
{
    ConvertIntToDecimalStringN(gStringVar1, Rogue_GetPhase() + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
}

u16 RogueScript_GetCurrentRouteVariant(void)
{
    return Rogue_GetCurrentRouteVariant();
}

u16 RogueScript_GetChapter(void)
{
    return Rogue_GetPhase();
}

u16 RogueScript_IsBossReady(void)
{
    return Rogue_IsRunActive() && Rogue_HasPendingRouteChoice()
        && Rogue_GetCurrentRoom() >= ROGUE_ROUTES_PER_CHAPTER;
}

u16 RogueScript_IsBossRoom(void)
{
    return Rogue_GetCurrentRoomType() == ROGUE_ROOM_BOSS;
}

u16 RogueScript_IsRunComplete(void)
{
    return Rogue_GetRunState() == ROGUE_RUN_COMPLETE;
}

u16 RogueScript_IsRunFailed(void)
{
    return Rogue_GetRunState() == ROGUE_RUN_FAILED;
}

void RogueScript_StartNewRun(void)
{
#if MODULE_ROGUE_ENABLED
    struct RogueSaveData *data = GetSaveData();

    if (data != NULL && (data->state == ROGUE_RUN_COMPLETE || data->state == ROGUE_RUN_FAILED))
    {
        ZeroPlayerPartyMons();
        ClearBag();
        StartRun(data, 0);
    }
#endif
}

u16 RogueScript_CollectPayout(void)
{
#if MODULE_ROGUE_ENABLED
    struct RogueSaveData *data = GetSaveData();
    u16 payout;

    if (data == NULL || data->pendingChips == 0)
        return 0;

    payout = data->pendingChips;
    data->rogueChips = min(MAX_COINS, data->rogueChips + payout);
    data->pendingChips = 0;
    ConvertIntToDecimalStringN(gStringVar1, payout, STR_CONV_MODE_LEFT_ALIGN, 5);
    ConvertIntToDecimalStringN(gStringVar2, data->rogueChips, STR_CONV_MODE_LEFT_ALIGN, 5);
    return payout;
#else
    return 0;
#endif
}

void RogueScript_BufferChipBalance(void)
{
#if MODULE_ROGUE_ENABLED
    struct RogueSaveData *data = GetSaveData();

    ConvertIntToDecimalStringN(gStringVar1, data != NULL ? data->rogueChips : 0, STR_CONV_MODE_LEFT_ALIGN, 5);
#else
    ConvertIntToDecimalStringN(gStringVar1, 0, STR_CONV_MODE_LEFT_ALIGN, 1);
#endif
}

void RogueScript_BeginCasinoSession(void)
{
#if MODULE_ROGUE_ENABLED
    struct RogueSaveData *data = GetSaveData();

    SetCoins(data != NULL ? min(data->rogueChips, MAX_COINS) : 0);
#else
    SetCoins(0);
#endif
}

void RogueScript_EndCasinoSession(void)
{
#if MODULE_ROGUE_ENABLED
    struct RogueSaveData *data = GetSaveData();

    if (data != NULL)
        data->rogueChips = GetCoins();
#endif
    SetCoins(0);
}

u16 RogueScript_TryPurchaseUnlock(void)
{
#if MODULE_ROGUE_ENABLED
    static const u16 sPrices[ROGUE_UNLOCK_COUNT] = {100, 75, 150};
    struct RogueSaveData *data = GetSaveData();
    u16 unlock = gSpecialVar_0x8004;
    u8 mask;

    if (data == NULL || unlock >= ROGUE_UNLOCK_COUNT)
        return ROGUE_PURCHASE_INVALID;

    mask = 1 << unlock;
    if (data->unlockFlags & mask)
        return ROGUE_PURCHASE_ALREADY_OWNED;
    if (data->rogueChips < sPrices[unlock])
        return ROGUE_PURCHASE_NOT_ENOUGH_CHIPS;

    data->rogueChips -= sPrices[unlock];
    data->unlockFlags |= mask;
    return ROGUE_PURCHASE_SUCCESS;
#else
    return ROGUE_PURCHASE_INVALID;
#endif
}

u16 RogueScript_HasBonusStarterUnlock(void)
{
#if MODULE_ROGUE_ENABLED
    struct RogueSaveData *data = GetSaveData();
    return data != NULL && (data->unlockFlags & (1 << ROGUE_UNLOCK_BONUS_STARTER));
#else
    return FALSE;
#endif
}

u16 RogueScript_HasMysteryPartnerUnlock(void)
{
#if MODULE_ROGUE_ENABLED
    struct RogueSaveData *data = GetSaveData();
    return data != NULL && (data->unlockFlags & (1 << ROGUE_UNLOCK_MYSTERY_PARTNER));
#else
    return FALSE;
#endif
}

u16 RogueScript_GrantStartingBundle(void)
{
#if MODULE_ROGUE_ENABLED
    struct RogueSaveData *data = GetSaveData();

    if (data == NULL
     || !(data->unlockFlags & (1 << ROGUE_UNLOCK_STARTING_BUNDLE))
     || data->bundleClaimedRun == data->runNumber)
        return FALSE;

    data->bundleClaimedRun = data->runNumber;
    AddBagItem(ITEM_POTION, 3);
    AddBagItem(ITEM_ANTIDOTE, 2);
    AddBagItem(ITEM_REVIVE, 1);
    return TRUE;
#else
    return FALSE;
#endif
}

u16 RogueScript_GiveMysteryPartner(void)
{
#if MODULE_ROGUE_ENABLED
    static const enum Species sMysteryPartners[] =
    {
        SPECIES_RALTS,
        SPECIES_SHINX,
        SPECIES_RIOLU,
        SPECIES_TOGEPI,
        SPECIES_DRATINI,
    };
    struct RogueSaveData *data = GetSaveData();
    enum Species species;

    if (data == NULL)
        return FALSE;

    species = sMysteryPartners[Rogue_RandomRange(ROGUE_RNG_REWARD, data->runNumber, ARRAY_COUNT(sMysteryPartners))];
    StringCopy(gStringVar1, GetSpeciesName(species));
    return ScriptGiveMonWithoutRandomization(species, 5, ITEM_NONE) != MON_CANT_GIVE;
#else
    return FALSE;
#endif
}
