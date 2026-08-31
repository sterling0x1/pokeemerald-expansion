#ifndef GUARD_ROGUE_H
#define GUARD_ROGUE_H

#include "global.h"
#include "config/modules.h"

#define ROGUE_SAVE_VERSION 3
#define ROGUE_MAX_ROUTE_CHOICES 3
#define ROGUE_ROUTES_PER_CHAPTER 2
#define ROGUE_CHAPTER_COUNT 4
#define ROGUE_ROUTE_VARIANT_COUNT 8

#define ROGUE_CHIPS_TRAINER_WIN  5
#define ROGUE_CHIPS_ROUTE_CLEAR 20
#define ROGUE_CHIPS_BOSS_CLEAR  75
#define ROGUE_CHIPS_DEFEAT      10
#define ROGUE_CHIPS_VICTORY    200

enum RogueUnlock
{
    ROGUE_UNLOCK_BONUS_STARTER,
    ROGUE_UNLOCK_STARTING_BUNDLE,
    ROGUE_UNLOCK_MYSTERY_PARTNER,
    ROGUE_UNLOCK_COUNT,
};

enum RoguePurchaseResult
{
    ROGUE_PURCHASE_SUCCESS,
    ROGUE_PURCHASE_ALREADY_OWNED,
    ROGUE_PURCHASE_NOT_ENOUGH_CHIPS,
    ROGUE_PURCHASE_INVALID,
};

enum RogueRunState
{
    ROGUE_RUN_INACTIVE,
    ROGUE_RUN_READY,
    ROGUE_RUN_ACTIVE,
    ROGUE_RUN_COMPLETE,
    ROGUE_RUN_FAILED,
};

enum RogueRandomScope
{
    ROGUE_RNG_RUN,
    ROGUE_RNG_PATH,
    ROGUE_RNG_ROOM,
    ROGUE_RNG_ENCOUNTER,
    ROGUE_RNG_TRAINER,
    ROGUE_RNG_REWARD,
    ROGUE_RNG_SHOP,
    ROGUE_RNG_COUNT,
};

enum RogueRoomType
{
    ROGUE_ROOM_START,
    ROGUE_ROOM_ROUTE,
    ROGUE_ROOM_TRAINER,
    ROGUE_ROOM_REST,
    ROGUE_ROOM_SHOP,
    ROGUE_ROOM_REWARD,
    ROGUE_ROOM_EVENT,
    ROGUE_ROOM_BOSS,
    ROGUE_ROOM_TRANSITION,
    ROGUE_ROOM_COUNT,
};

struct RogueRoomChoice
{
    u8 type;
    u8 difficulty;
    u16 routeKey;
};

struct RogueSaveData
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
    u8 unlockFlags;
    u16 rogueChips;
    u16 pendingChips;
    u16 bundleClaimedRun;
    u8 reserved[2];
};

#if MODULE_ROGUE_ENABLED
void Rogue_InitNewSave(void);
void Rogue_LoadSave(void);
bool32 Rogue_IsModeActive(void);
bool32 Rogue_IsRunActive(void);
enum RogueRunState Rogue_GetRunState(void);
void Rogue_BeginRun(u32 seed);
void Rogue_EndRun(bool32 victory);
u32 Rogue_GetBaseSeed(void);
u32 Rogue_GetRunSeed(void);
u32 Rogue_Random(enum RogueRandomScope scope, u32 key);
u32 Rogue_RandomRange(enum RogueRandomScope scope, u32 key, u32 range);
u8 Rogue_GetPhase(void);
u8 Rogue_GetCurrentRoom(void);
u8 Rogue_GetRoomCount(void);
u8 Rogue_GetCurrentRouteVariant(void);
enum RogueRoomType Rogue_GetCurrentRoomType(void);
bool32 Rogue_HasPendingRouteChoice(void);
u8 Rogue_GetRouteChoices(struct RogueRoomChoice *choices, u8 capacity);
bool32 Rogue_SelectRouteChoice(u8 choiceIndex);
bool32 Rogue_CompleteCurrentRoom(void);
void Rogue_HandleWhiteOut(void);
void Rogue_RecordTrainerWin(void);
#else
static inline void Rogue_InitNewSave(void) {}
static inline void Rogue_LoadSave(void) {}
static inline bool32 Rogue_IsModeActive(void) { return FALSE; }
static inline bool32 Rogue_IsRunActive(void) { return FALSE; }
static inline enum RogueRunState Rogue_GetRunState(void) { return ROGUE_RUN_INACTIVE; }
static inline void Rogue_BeginRun(u32 seed) { (void)seed; }
static inline void Rogue_EndRun(bool32 victory) { (void)victory; }
static inline u32 Rogue_GetBaseSeed(void) { return 0; }
static inline u32 Rogue_GetRunSeed(void) { return 0; }
static inline u32 Rogue_Random(enum RogueRandomScope scope, u32 key) { (void)scope; (void)key; return 0; }
static inline u32 Rogue_RandomRange(enum RogueRandomScope scope, u32 key, u32 range) { (void)scope; (void)key; (void)range; return 0; }
static inline u8 Rogue_GetPhase(void) { return 0; }
static inline u8 Rogue_GetCurrentRoom(void) { return 0; }
static inline u8 Rogue_GetRoomCount(void) { return 0; }
static inline u8 Rogue_GetCurrentRouteVariant(void) { return 0; }
static inline enum RogueRoomType Rogue_GetCurrentRoomType(void) { return ROGUE_ROOM_START; }
static inline bool32 Rogue_HasPendingRouteChoice(void) { return FALSE; }
static inline u8 Rogue_GetRouteChoices(struct RogueRoomChoice *choices, u8 capacity) { (void)choices; (void)capacity; return 0; }
static inline bool32 Rogue_SelectRouteChoice(u8 choiceIndex) { (void)choiceIndex; return FALSE; }
static inline bool32 Rogue_CompleteCurrentRoom(void) { return FALSE; }
static inline void Rogue_HandleWhiteOut(void) {}
static inline void Rogue_RecordTrainerWin(void) {}
#endif

// Event-script bridges used by Rogue maps.
u16 RogueScript_SelectFirstRoute(void);
u16 RogueScript_CompleteRoom(void);
void RogueScript_BufferNextRoomNumber(void);
void RogueScript_BufferChapterNumber(void);
u16 RogueScript_GetCurrentRouteVariant(void);
u16 RogueScript_GetChapter(void);
u16 RogueScript_IsBossReady(void);
u16 RogueScript_IsBossRoom(void);
u16 RogueScript_IsRunComplete(void);
u16 RogueScript_IsRunFailed(void);
void RogueScript_StartNewRun(void);
u16 RogueScript_CollectPayout(void);
void RogueScript_BufferChipBalance(void);
void RogueScript_BeginCasinoSession(void);
void RogueScript_EndCasinoSession(void);
u16 RogueScript_TryPurchaseUnlock(void);
u16 RogueScript_HasBonusStarterUnlock(void);
u16 RogueScript_HasMysteryPartnerUnlock(void);
u16 RogueScript_GrantStartingBundle(void);
u16 RogueScript_GiveMysteryPartner(void);

#endif // GUARD_ROGUE_H
