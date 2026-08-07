#include "global.h"
#include "active_battle.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_interface.h"
#include "bg.h"
#include "main.h"
#include "menu.h"
#include "module_save.h"
#include "move.h"
#include "random.h"
#include "international_string_util.h"
#include "text.h"
#include "window.h"
#include "constants/battle.h"
#include "constants/moves.h"

#if MODULE_ACTIVE_BATTLE_ENABLED

#define ACTIVE_BATTLE_SAVE_VERSION 1
#define METER_X 12
#define METER_Y 18
#define METER_WIDTH 184
#define METER_HEIGHT 6
#define OPENING_LOCK_FRAMES 6
#define PROMPT_BG_COLOR 0xF
#define DODGE_RING_CENTER_X 16
#define DODGE_RING_CENTER_Y 16
#define DODGE_RING_MAX_RADIUS 14
#define DODGE_START_DELAY_FRAMES 24
#define PROMPT_BLANK_TILE 0x39F

enum ActiveBattlePromptState
{
    PROMPT_IDLE,
    PROMPT_RUNNING,
    PROMPT_RESOLVED,
};

enum ActiveBattlePromptType
{
    PROMPT_TYPE_NONE,
    PROMPT_TYPE_CRITICAL,
    PROMPT_TYPE_DODGE,
};

struct ActiveBattlePrompt
{
    u8 state;
    u8 frame;
    u8 marker;
    u8 target;
    u8 result;
    u8 type;
    u8 windowId;
    bool8 windowActive;
    u8 windowLeft;
    u8 windowTop;
};

static EWRAM_DATA struct ActiveBattlePrompt sPrompt = {0};
static const u8 sDodgeOverlayText[] = _("R");
static const u8 sCriticalOverlayText[] = _("CRIT");
static const u8 sDodgeTextColors[] = {TEXT_COLOR_TRANSPARENT, 1, 6};
static const u32 sPromptBlankTile[8] = {0};

static const struct WindowTemplate sPromptWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 0,
    .tilemapTop = 0,
    .width = 4,
    .height = 4,
    .paletteNum = 0,
    .baseBlock = 0x3A0,
};

static struct ActiveBattleSaveData *GetSaveData(void)
{
    u16 version;
    u16 size;
    struct ActiveBattleSaveData *data = ModuleSave_GetChunk(MODULE_ID_ACTIVE_BATTLE, &version, &size);

    if (data == NULL || version != ACTIVE_BATTLE_SAVE_VERSION || size != sizeof(*data))
        return NULL;
    return data;
}

static void SetDefaults(struct ActiveBattleSaveData *data)
{
    data->criticals = FALSE;
    data->dodging = FALSE;
    data->timingDifficulty = ACTIVE_BATTLE_TIMING_STANDARD;
    data->reserved = 0;
}

void ActiveBattle_InitNewSave(void)
{
    struct ActiveBattleSaveData *data = ModuleSave_RecreateChunk(MODULE_ID_ACTIVE_BATTLE, ACTIVE_BATTLE_SAVE_VERSION, sizeof(*data));

    if (data != NULL)
        SetDefaults(data);
}

void ActiveBattle_LoadSave(void)
{
    struct ActiveBattleSaveData *data = GetSaveData();

    if (data == NULL)
    {
        data = ModuleSave_RecreateChunk(MODULE_ID_ACTIVE_BATTLE, ACTIVE_BATTLE_SAVE_VERSION, sizeof(*data));
        if (data != NULL)
            SetDefaults(data);
    }
    else
    {
        data->criticals = !!data->criticals;
        data->dodging = !!data->dodging;
        if (data->timingDifficulty >= ACTIVE_BATTLE_TIMING_COUNT)
            data->timingDifficulty = ACTIVE_BATTLE_TIMING_STANDARD;
    }
}

bool32 ActiveBattle_AreCriticalsEnabled(void)
{
    struct ActiveBattleSaveData *data;

    ActiveBattle_LoadSave();
    data = GetSaveData();
    return data != NULL && data->criticals;
}

void ActiveBattle_SetCriticalsEnabled(bool32 enabled)
{
    struct ActiveBattleSaveData *data;

    ActiveBattle_LoadSave();
    data = GetSaveData();
    if (data != NULL)
        data->criticals = !!enabled;
}

bool32 ActiveBattle_IsDodgingEnabled(void)
{
    struct ActiveBattleSaveData *data;

    ActiveBattle_LoadSave();
    data = GetSaveData();
    return data != NULL && data->dodging;
}

void ActiveBattle_SetDodgingEnabled(bool32 enabled)
{
    struct ActiveBattleSaveData *data;

    ActiveBattle_LoadSave();
    data = GetSaveData();
    if (data != NULL)
        data->dodging = !!enabled;
}

enum ActiveBattleTimingDifficulty ActiveBattle_GetTimingDifficulty(void)
{
    struct ActiveBattleSaveData *data;

    ActiveBattle_LoadSave();
    data = GetSaveData();
    return data == NULL ? ACTIVE_BATTLE_TIMING_STANDARD : data->timingDifficulty;
}

void ActiveBattle_SetTimingDifficulty(enum ActiveBattleTimingDifficulty difficulty)
{
    struct ActiveBattleSaveData *data;

    ActiveBattle_LoadSave();
    data = GetSaveData();
    if (data != NULL)
        data->timingDifficulty = difficulty < ACTIVE_BATTLE_TIMING_COUNT ? difficulty : ACTIVE_BATTLE_TIMING_STANDARD;
}

static bool32 IsFixedDamageMove(enum Move move);

static bool32 IsEligibleMove(enum BattlerId attacker, enum BattlerId target, enum Move move)
{
const u32 excludedBattles = BATTLE_TYPE_LINK | BATTLE_TYPE_MULTI
                          | BATTLE_TYPE_SAFARI | BATTLE_TYPE_FIRST_BATTLE | BATTLE_TYPE_CATCH_TUTORIAL
                          | BATTLE_TYPE_INGAME_PARTNER | BATTLE_TYPE_RECORDED | BATTLE_TYPE_RECORDED_LINK
                          | BATTLE_TYPE_TRAINER_HILL | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_POKEDUDE
                          | BATTLE_TYPE_RECORDED_IS_MASTER;

    if (gBattleTypeFlags & excludedBattles)
        return FALSE;
    if (attacker >= gBattlersCount || target >= gBattlersCount)
        return FALSE;
    if (GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS || GetMovePower(move) == 0)
        return FALSE;
    if (IsSpreadMove(GetBattlerMoveTargetType(attacker, move)) || IsMultiHitMove(move))
        return FALSE;
    if (IsFixedDamageMove(move))
        return FALSE;
    return TRUE;
}

static enum ActiveBattlePromptType GetPromptType(enum BattlerId attacker, enum BattlerId target, enum Move move)
{
    if (!IsEligibleMove(attacker, target, move))
        return PROMPT_TYPE_NONE;
    if (IsOnPlayerSide(attacker) && ActiveBattle_AreCriticalsEnabled())
        return PROMPT_TYPE_CRITICAL;
    if (!IsOnPlayerSide(attacker) && IsOnPlayerSide(target) && ActiveBattle_IsDodgingEnabled())
        return PROMPT_TYPE_DODGE;
    return PROMPT_TYPE_NONE;
}

static u8 GetSpeed(void)
{
    switch (ActiveBattle_GetTimingDifficulty())
    {
    case ACTIVE_BATTLE_TIMING_EXPERT:
        return 5;
    case ACTIVE_BATTLE_TIMING_HARD:
        return 4;
    case ACTIVE_BATTLE_TIMING_STANDARD:
    default:
        return 3;
    }
}

static u16 GetCriticalTimeoutFrames(void)
{
    const u16 cycleLength = (METER_WIDTH - 1) * 2;

    return (cycleLength + GetSpeed() - 1) / GetSpeed();
}

static u8 GetDodgeSpeed(void);
static u8 GetDodgeStepInterval(void);

static u16 GetDodgeTimeoutFrames(void)
{
    u8 speed = GetDodgeSpeed();

    return DODGE_START_DELAY_FRAMES
         + ((DODGE_RING_MAX_RADIUS + speed - 1) / speed) * GetDodgeStepInterval();
}

static u8 GetGoodRadius(void)
{
    return ActiveBattle_GetTimingDifficulty() == ACTIVE_BATTLE_TIMING_STANDARD ? 15
         : ActiveBattle_GetTimingDifficulty() == ACTIVE_BATTLE_TIMING_HARD ? 11
         : 8;
}

static u8 GetPerfectRadius(void)
{
    return ActiveBattle_GetTimingDifficulty() == ACTIVE_BATTLE_TIMING_STANDARD ? 5
         : ActiveBattle_GetTimingDifficulty() == ACTIVE_BATTLE_TIMING_HARD ? 4
         : 3;
}

static u8 GetDodgeSpeed(void)
{
    return ActiveBattle_GetTimingDifficulty() == ACTIVE_BATTLE_TIMING_EXPERT ? 2 : 1;
}

static u8 GetDodgeStepInterval(void)
{
    return ActiveBattle_GetTimingDifficulty() == ACTIVE_BATTLE_TIMING_STANDARD ? 2 : 1;
}

static u8 GetDodgeGoodRadius(void)
{
    return ActiveBattle_GetTimingDifficulty() == ACTIVE_BATTLE_TIMING_STANDARD ? 2
         : ActiveBattle_GetTimingDifficulty() == ACTIVE_BATTLE_TIMING_HARD ? 1
         : 0;
}

static u8 GetDodgePerfectRadius(void)
{
    return 0;
}

static u8 GetDodgeTarget(void)
{
    return ActiveBattle_GetTimingDifficulty() == ACTIVE_BATTLE_TIMING_EXPERT ? 4 : 5;
}

static bool32 IsFixedDamageMove(enum Move move)
{
    switch (GetMoveEffect(move))
    {
    case EFFECT_LEVEL_DAMAGE:
    case EFFECT_PSYWAVE:
    case EFFECT_FIXED_HP_DAMAGE:
    case EFFECT_FIXED_PERCENT_DAMAGE:
    case EFFECT_FINAL_GAMBIT:
    case EFFECT_REFLECT_DAMAGE:
    case EFFECT_ENDEAVOR:
    case EFFECT_OHKO:
    case EFFECT_BIDE:
        return TRUE;
    default:
        return FALSE;
    }
}

static bool32 CreatePromptOverlay(enum BattlerId battler)
{
    struct WindowTemplate template = sPromptWindowTemplate;
    s16 centerX;
    s16 centerY;
    s16 spriteLeft;
    s16 spriteRight;
    s16 spriteTop;
    s16 spriteBottom;

    if (sPrompt.windowActive)
        return TRUE;

    spriteLeft = GetBattlerSpriteCoordAttr(battler, BATTLER_COORD_ATTR_LEFT);
    spriteRight = GetBattlerSpriteCoordAttr(battler, BATTLER_COORD_ATTR_RIGHT);
    spriteTop = GetBattlerSpriteCoordAttr(battler, BATTLER_COORD_ATTR_TOP);
    spriteBottom = GetBattlerSpriteCoordAttr(battler, BATTLER_COORD_ATTR_BOTTOM);

    // Keep the prompt clear of the Pokemon while remaining visually tied to it.
    centerX = GetBattlerSide(battler) == B_SIDE_PLAYER ? spriteRight + 18 : spriteLeft - 18;
    centerY = (spriteTop + spriteBottom) / 2;
    centerX = max(16, min(centerX, 224));
    centerY = max(16, min(centerY, 104));
    template.tilemapLeft = (centerX - 16) / 8;
    template.tilemapTop = (centerY - 16) / 8;
    sPrompt.windowLeft = template.tilemapLeft;
    sPrompt.windowTop = template.tilemapTop;

    LoadBgTiles(0, sPromptBlankTile, sizeof(sPromptBlankTile), PROMPT_BLANK_TILE);
    sPrompt.windowId = AddWindow(&template);
    if (sPrompt.windowId == WINDOW_NONE)
        return FALSE;
    sPrompt.windowActive = TRUE;

    FillWindowPixelBuffer(sPrompt.windowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    PutWindowTilemap(sPrompt.windowId);
    CopyWindowToVram(sPrompt.windowId, COPYWIN_FULL);
    return TRUE;
}

static void DestroyPromptOverlay(void)
{
    if (!sPrompt.windowActive)
        return;

    FillWindowPixelBuffer(sPrompt.windowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    CopyWindowToVram(sPrompt.windowId, COPYWIN_GFX);
    FillBgTilemapBufferRect(0, PROMPT_BLANK_TILE, sPrompt.windowLeft, sPrompt.windowTop, 4, 4, 0);
    ScheduleBgCopyTilemapToVram(0);
    RemoveWindow(sPrompt.windowId);
    sPrompt.windowActive = FALSE;
}

static void DrawPromptRing(u8 radius, u8 color)
{
    s16 x = radius;
    s16 y = 0;
    s16 decision = 1 - x;

    while (y <= x)
    {
        FillWindowPixelRect(sPrompt.windowId, PIXEL_FILL(color), DODGE_RING_CENTER_X + x - 1, DODGE_RING_CENTER_Y + y - 1, 2, 2);
        FillWindowPixelRect(sPrompt.windowId, PIXEL_FILL(color), DODGE_RING_CENTER_X + y - 1, DODGE_RING_CENTER_Y + x - 1, 2, 2);
        FillWindowPixelRect(sPrompt.windowId, PIXEL_FILL(color), DODGE_RING_CENTER_X - y - 1, DODGE_RING_CENTER_Y + x - 1, 2, 2);
        FillWindowPixelRect(sPrompt.windowId, PIXEL_FILL(color), DODGE_RING_CENTER_X - x - 1, DODGE_RING_CENTER_Y + y - 1, 2, 2);
        FillWindowPixelRect(sPrompt.windowId, PIXEL_FILL(color), DODGE_RING_CENTER_X - x - 1, DODGE_RING_CENTER_Y - y - 1, 2, 2);
        FillWindowPixelRect(sPrompt.windowId, PIXEL_FILL(color), DODGE_RING_CENTER_X - y - 1, DODGE_RING_CENTER_Y - x - 1, 2, 2);
        FillWindowPixelRect(sPrompt.windowId, PIXEL_FILL(color), DODGE_RING_CENTER_X + y - 1, DODGE_RING_CENTER_Y - x - 1, 2, 2);
        FillWindowPixelRect(sPrompt.windowId, PIXEL_FILL(color), DODGE_RING_CENTER_X + x - 1, DODGE_RING_CENTER_Y - y - 1, 2, 2);

        y++;
        if (decision <= 0)
        {
            decision += 2 * y + 1;
        }
        else
        {
            x--;
            decision += 2 * (y - x) + 1;
        }
    }
}

static void DrawCriticalPrompt(void)
{
    u8 markerRadius = 3 + (sPrompt.marker * 11) / (METER_WIDTH - 1);
    u8 targetRadius = 3 + (sPrompt.target * 11) / (METER_WIDTH - 1);

    FillWindowPixelBuffer(sPrompt.windowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    DrawPromptRing(targetRadius, TEXT_COLOR_LIGHT_GRAY);
    DrawPromptRing(markerRadius, TEXT_COLOR_DARK_GRAY);
    AddTextPrinterParameterized4(sPrompt.windowId, FONT_SMALL_NARROWER, 5, 9, 0, 0,
                                 sDodgeTextColors, TEXT_SKIP_DRAW, sCriticalOverlayText);
    CopyWindowToVram(sPrompt.windowId, COPYWIN_GFX);
}

static void DrawDodgePrompt(void)
{
    FillWindowPixelBuffer(sPrompt.windowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    DrawPromptRing(sPrompt.target, TEXT_COLOR_LIGHT_GRAY);
    DrawPromptRing(sPrompt.marker, TEXT_COLOR_DARK_GRAY);
    AddTextPrinterParameterized4(sPrompt.windowId, FONT_SMALL_NARROWER, 13, 9, 0, 0,
                                 sDodgeTextColors, TEXT_SKIP_DRAW, sDodgeOverlayText);
    CopyWindowToVram(sPrompt.windowId, COPYWIN_GFX);
}

static void ResolvePrompt(void)
{
    u8 distance = abs((s16)sPrompt.marker - (s16)sPrompt.target);
    u8 perfectRadius = sPrompt.type == PROMPT_TYPE_DODGE ? GetDodgePerfectRadius() : GetPerfectRadius();
    u8 goodRadius = sPrompt.type == PROMPT_TYPE_DODGE ? GetDodgeGoodRadius() : GetGoodRadius();

    if (distance <= perfectRadius)
        sPrompt.result = ACTIVE_BATTLE_CRIT_PERFECT;
    else if (distance <= goodRadius)
        sPrompt.result = ACTIVE_BATTLE_CRIT_GOOD;
    else
        sPrompt.result = ACTIVE_BATTLE_CRIT_NONE;
    sPrompt.state = PROMPT_RESOLVED;
}

bool32 ActiveBattle_IsPromptActive(void)
{
    return sPrompt.state != PROMPT_IDLE;
}

bool32 ActiveBattle_UpdateDamagePrompt(enum BattlerId attacker, enum BattlerId target, enum Move move)
{
    u16 phase;

    if (sPrompt.state == PROMPT_RESOLVED)
        return TRUE;
if (sPrompt.state == PROMPT_IDLE)
{
    u8 previousTarget = sPrompt.target;
    enum ActiveBattlePromptType promptType = GetPromptType(attacker, target, move);
    enum BattlerId promptBattler;

    if (promptType == PROMPT_TYPE_NONE)
        return TRUE;

    promptBattler = promptType == PROMPT_TYPE_CRITICAL
                  ? attacker
                  : target;

    if (!CreatePromptOverlay(promptBattler))
        return TRUE;
        sPrompt.state = PROMPT_RUNNING;
        sPrompt.type = promptType;
        sPrompt.frame = 0;
        if (promptType == PROMPT_TYPE_CRITICAL)
        {
            sPrompt.marker = 0;
            sPrompt.target = 28 + (Random2() % 128);
            if (sPrompt.target == previousTarget)
                sPrompt.target = 28 + ((sPrompt.target - 28 + 37) % 128);
        }
        else
        {
            sPrompt.marker = DODGE_RING_MAX_RADIUS;
            sPrompt.target = GetDodgeTarget();
        }
        sPrompt.result = ACTIVE_BATTLE_CRIT_NONE;
    }

    sPrompt.frame++;
    if (sPrompt.type == PROMPT_TYPE_CRITICAL)
    {
        phase = sPrompt.frame * GetSpeed();
        phase %= (METER_WIDTH - 1) * 2;
        sPrompt.marker = phase < METER_WIDTH ? phase : (METER_WIDTH - 1) * 2 - phase;
        DrawCriticalPrompt();
    }
    else
    {
        phase = sPrompt.frame > DODGE_START_DELAY_FRAMES
              ? ((sPrompt.frame - DODGE_START_DELAY_FRAMES) / GetDodgeStepInterval()) * GetDodgeSpeed()
              : 0;
        sPrompt.marker = phase < DODGE_RING_MAX_RADIUS ? DODGE_RING_MAX_RADIUS - phase : 0;
        DrawDodgePrompt();
    }

    if (sPrompt.frame > OPENING_LOCK_FRAMES && JOY_NEW(R_BUTTON))
        ResolvePrompt();
    else if ((sPrompt.type == PROMPT_TYPE_CRITICAL && sPrompt.frame >= GetCriticalTimeoutFrames())
          || (sPrompt.type == PROMPT_TYPE_DODGE && sPrompt.frame >= GetDodgeTimeoutFrames()))
        ResolvePrompt();

    return sPrompt.state == PROMPT_RESOLVED;
}

enum ActiveBattleCriticalResult ActiveBattle_GetCriticalResult(void)
{
    return sPrompt.state == PROMPT_RESOLVED && sPrompt.type == PROMPT_TYPE_CRITICAL
         ? sPrompt.result
         : ACTIVE_BATTLE_CRIT_NONE;
}

s32 ActiveBattle_AdjustDamageForDodge(s32 damage)
{
    if (sPrompt.state != PROMPT_RESOLVED || sPrompt.type != PROMPT_TYPE_DODGE || damage <= 0)
        return damage;
    if (sPrompt.result == ACTIVE_BATTLE_DODGE_PERFECT)
    {
        gBattleStruct->moveResultFlags[gBattlerTarget] |= MOVE_RESULT_MISSED | MOVE_RESULT_AVOIDED_ATTACK;
        gSpecialStatuses[gBattlerTarget].criticalHit = FALSE;
        return 0;
    }
    if (sPrompt.result == ACTIVE_BATTLE_DODGE_GOOD)
        return max(1, damage / 2);
    return damage;
}

void ActiveBattle_FinishDamageCalc(void)
{
    if (sPrompt.type == PROMPT_TYPE_CRITICAL || sPrompt.type == PROMPT_TYPE_DODGE)
        DestroyPromptOverlay();
    sPrompt.state = PROMPT_IDLE;
    sPrompt.type = PROMPT_TYPE_NONE;
    sPrompt.result = ACTIVE_BATTLE_CRIT_NONE;
}

#endif // MODULE_ACTIVE_BATTLE_ENABLED
