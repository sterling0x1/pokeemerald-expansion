#ifndef GUARD_ROGUE_H
#define GUARD_ROGUE_H

#include "global.h"
#include "config/modules.h"

#define ROGUE_SAVE_VERSION 1

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
    u8 reserved[9];
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
#endif

#endif // GUARD_ROGUE_H
