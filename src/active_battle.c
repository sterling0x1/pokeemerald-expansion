#include "global.h"
#include "active_battle.h"
#include "battle.h"
#include "battle_interface.h"
#include "battle_message.h"
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
};

static EWRAM_DATA struct ActiveBattlePrompt sPrompt = {0};
static const u8 sPromptText[] = _("CRITICAL TIMING");
static const u8 sPromptControl[] = _("{R_BUTTON} STOP");
static const u8 sDodgeOverlayText[] = _("R");
static const u8 sPromptTextColors[] = {PROMPT_BG_COLOR, 1, 6};
static const u8 sDodgeTextColors[] = {TEXT_COLOR_TRANSPARENT, 1, 6};

static const struct WindowTemplate sDodgeWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 7,
    .tilemapTop = 9,
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
    const u32 excludedBattles = BATTLE_TYPE_DOUBLE | BATTLE_TYPE_LINK | BATTLE_TYPE_MULTI
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

static void DrawCriticalPrompt(void)
{
    s16 goodLeft = max(0, sPrompt.target - GetGoodRadius());
    s16 goodRight = min(METER_WIDTH - 1, sPrompt.target + GetGoodRadius());
    s16 perfectLeft = max(0, sPrompt.target - GetPerfectRadius());
    s16 perfectRight = min(METER_WIDTH - 1, sPrompt.target + GetPerfectRadius());

    if (sPrompt.frame == 1)
    {
        u8 titleX = GetStringCenterAlignXOffset(FONT_SMALL_NARROWER, sPromptText, 128) + 8;

        FillWindowPixelBuffer(B_WIN_MSG, PIXEL_FILL(PROMPT_BG_COLOR));
        AddTextPrinterParameterized4(B_WIN_MSG, FONT_SMALL_NARROWER, titleX, 2, 0, 0,
                                     sPromptTextColors, TEXT_SKIP_DRAW, sPromptText);
        AddTextPrinterParameterized4(B_WIN_MSG, FONT_SMALL_NARROWER, 154, 2, 0, 0,
                                     sPromptTextColors, TEXT_SKIP_DRAW, sPromptControl);
    }

    FillWindowPixelRect(B_WIN_MSG, PIXEL_FILL(TEXT_COLOR_DARK_GRAY), METER_X - 2, METER_Y - 2, METER_WIDTH + 4, METER_HEIGHT + 4);
    FillWindowPixelRect(B_WIN_MSG, PIXEL_FILL(TEXT_COLOR_LIGHT_GRAY), METER_X, METER_Y, METER_WIDTH, METER_HEIGHT);
    FillWindowPixelRect(B_WIN_MSG, PIXEL_FILL(TEXT_COLOR_RED), METER_X + goodLeft, METER_Y, goodRight - goodLeft + 1, METER_HEIGHT);
    FillWindowPixelRect(B_WIN_MSG, PIXEL_FILL(TEXT_COLOR_GREEN), METER_X + perfectLeft, METER_Y, perfectRight - perfectLeft + 1, METER_HEIGHT);
    FillWindowPixelRect(B_WIN_MSG, PIXEL_FILL(TEXT_COLOR_WHITE), METER_X + sPrompt.marker, METER_Y - 2, 2, METER_HEIGHT + 4);
    CopyWindowToVram(B_WIN_MSG, COPYWIN_GFX);
}

static bool32 CreateDodgeOverlay(void)
{
    if (sPrompt.windowActive)
        return TRUE;

    sPrompt.windowId = AddWindow(&sDodgeWindowTemplate);
    if (sPrompt.windowId == WINDOW_NONE)
        return FALSE;
    sPrompt.windowActive = TRUE;

    FillWindowPixelBuffer(sPrompt.windowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    PutWindowTilemap(sPrompt.windowId);
    CopyWindowToVram(sPrompt.windowId, COPYWIN_FULL);
    return TRUE;
}

static void DestroyDodgeOverlay(void)
{
    if (!sPrompt.windowActive)
        return;

    FillWindowPixelBuffer(sPrompt.windowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    CopyWindowToVram(sPrompt.windowId, COPYWIN_GFX);
    RemoveWindow(sPrompt.windowId);
    sPrompt.windowActive = FALSE;
}

static void DrawDodgeRing(u8 radius, u8 color)
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

static void DrawDodgePrompt(void)
{
    FillWindowPixelBuffer(sPrompt.windowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    DrawDodgeRing(sPrompt.target, TEXT_COLOR_GREEN);
    DrawDodgeRing(sPrompt.marker, TEXT_COLOR_RED);
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

bool32 ActiveBattle_UpdateDamagePrompt(enum BattlerId attacker, enum BattlerId target, enum Move move)
{
    u16 phase;

    if (sPrompt.state == PROMPT_RESOLVED)
        return TRUE;
    if (sPrompt.state == PROMPT_IDLE)
    {
        u8 previousTarget = sPrompt.target;
        enum ActiveBattlePromptType promptType = GetPromptType(attacker, target, move);

        if (promptType == PROMPT_TYPE_NONE)
            return TRUE;
        if (promptType == PROMPT_TYPE_DODGE && !CreateDodgeOverlay())
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
    if (sPrompt.state != PROMPT_IDLE && sPrompt.type == PROMPT_TYPE_CRITICAL)
        BattlePutTextOnWindow(gText_EmptyString3, B_WIN_MSG);
    else if (sPrompt.type == PROMPT_TYPE_DODGE)
    {
        DestroyDodgeOverlay();
    }
    sPrompt.state = PROMPT_IDLE;
    sPrompt.type = PROMPT_TYPE_NONE;
    sPrompt.result = ACTIVE_BATTLE_CRIT_NONE;
}

#endif // MODULE_ACTIVE_BATTLE_ENABLED
