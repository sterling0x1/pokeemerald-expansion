#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_arena.h"
#include "battle_controllers.h"
#include "battle_dome.h"
#include "battle_interface.h"
#include "battle_message.h"
#include "battle_script_commands.h"
#include "battle_setup.h"
#include "battle_tv.h"
#include "battle_util.h"
#include "battle_z_move.h"
#include "battle_gimmick.h"
#include "bg.h"
#include "data.h"
#include "extended_options.h"
#include "item.h"
#include "item_menu.h"
#include "item_use.h"
#include "link.h"
#include "main.h"
#include "nuzlocke.h"
#include "m4a.h"
#include "palette.h"
#include "party_menu.h"
#include "pokeball.h"
#include "pokemon.h"
#include "random.h"
#include "recorded_battle.h"
#include "reshow_battle_screen.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "test_runner.h"
#include "text.h"
#include "trainer.h"
#include "util.h"
#include "window.h"
#include "line_break.h"
#include "constants/battle_anim.h"
#include "constants/battle_move_effects.h"
#include "constants/battle_partner.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/party_menu.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/rgb.h"
#include "caps.h"
#include "menu.h"
#include "pokemon_summary_screen.h"
#include "type_icons.h"
#include "pokedex.h"
#include "test/battle.h"

static const u8 sCompactMoveTextColors[] = {TEXT_COLOR_TRANSPARENT, 13, 15};
#define COMPACT_ARROW_RED   1
#define COMPACT_ARROW_GREEN 6
#define COMPACT_ARROW_GRAY  11

// Both player controllers can be choosing commands at the same time in a
// double battle. Keep compact-menu state per battler so opening one battler's
// action menu cannot overwrite the other battler's move/picker state.
static bool8 sUsingCompactMoveList[MAX_BATTLERS_COUNT];
// This screen can be open for either player battler in a double battle.
// Keep its return destination per battler so one battler's move screen never
// changes how the other battler's picker completes.
static bool8 sCompactAttackerPickerFromMoveList[MAX_BATTLERS_COUNT];
static bool8 sUsingModernActionMenu;
// The enemy-info pane is static while the command cursor moves. Redrawing it
// every input frame caused a visible flash on the right side of the screen.
static bool8 sModernActionMenuPromptNeedsRefresh;
static u8 sCompactAttackerCursor[MAX_BATTLERS_COUNT];
static u8 sCompactAttackerNames[PARTY_SIZE][POKEMON_NAME_BUFFER_SIZE];
static bool8 sBlockCompactAUntilReleased[MAX_BATTLERS_COUNT];
static EWRAM_DATA u8 sCompactAttackerStatusSpriteIds[MAX_BATTLERS_COUNT];
#if MODULE_BATTLE_BAG_ENABLED
static u8 sBattleBagCursor[MAX_BATTLERS_COUNT];
static u8 sBattleBagPocketPos[MAX_BATTLERS_COUNT];
static u8 sBattleBagListTop[MAX_BATTLERS_COUNT];
static bool8 sBattleBagBrowsingItems[MAX_BATTLERS_COUNT];
static bool8 sBattleBagPocketEmpty[MAX_BATTLERS_COUNT];
static bool8 sBattleBagBlockAUntilReleased[MAX_BATTLERS_COUNT];
static bool8 sBattleBagItemPending[MAX_BATTLERS_COUNT];
static enum Item sBattleBagPendingItem[MAX_BATTLERS_COUNT];
#endif

// Ordered so double-battle comparisons can keep the strongest visible result.
enum
{
    EFFECTIVENESS_CANNOT_VIEW,
    EFFECTIVENESS_NO_EFFECT,
    EFFECTIVENESS_NOT_VERY_EFFECTIVE,
    EFFECTIVENESS_NORMAL,
    EFFECTIVENESS_SUPER_EFFECTIVE,
};
static void PlayerHandleLoadMonSprite(enum BattlerId battler);
static void PlayerHandleDrawTrainerPic(enum BattlerId battler);
static void PlayerHandleTrainerSlide(enum BattlerId battler);
static void PlayerHandleTrainerSlideBack(enum BattlerId battler);
static void PlayerHandlePaletteFade(enum BattlerId battler);
static void PlayerHandlePause(enum BattlerId battler);
static void PlayerHandleChooseAction(enum BattlerId battler);
static void PlayerHandleYesNoBox(enum BattlerId battler);
static void PlayerHandleChooseItem(enum BattlerId battler);
static void PlayerHandleChoosePokemon(enum BattlerId battler);
static void PlayerHandleCmd23(enum BattlerId battler);
static void PlayerHandleStatusXor(enum BattlerId battler);
static void PlayerHandleDMA3Transfer(enum BattlerId battler);
static void PlayerHandlePlayBGM(enum BattlerId battler);
static void PlayerHandleTwoReturnValues(enum BattlerId battler);
static void PlayerHandleChosenMonReturnValue(enum BattlerId battler);
static void PlayerHandleOneReturnValue(enum BattlerId battler);
static void PlayerHandleOneReturnValue_Duplicate(enum BattlerId battler);
static void PlayerHandleIntroTrainerBallThrow(enum BattlerId battler);
static void PlayerHandleDrawPartyStatusSummary(enum BattlerId battler);
static void PlayerHandleEndBounceEffect(enum BattlerId battler);
static void PlayerHandleLinkStandbyMsg(enum BattlerId battler);
static void PlayerHandleResetActionMoveSelection(enum BattlerId battler);
static void PlayerHandleEndLinkBattle(enum BattlerId battler);
static void PlayerHandleBattleDebug(enum BattlerId battler);

static void PlayerBufferRunCommand(enum BattlerId battler);
static void MoveSelectionDisplayPPNumber(enum BattlerId battler);
static void MoveSelectionDisplayPPString(enum BattlerId battler);
static void MoveSelectionDisplayMoveType(enum BattlerId battler);
static void MoveSelectionDisplayMoveNames(enum BattlerId battler);
static void DrawCompactMoveList(enum BattlerId battler);
static void DrawCompactMoveInfo(enum BattlerId battler);
static void TryMoveSelectionDisplayMoveDescription(enum BattlerId battler);
static void MoveSelectionDisplayMoveDescription(enum BattlerId battler);
static void DrawModernMoveSelectionPanels(void);
static void OpenCompactAttackerPicker(enum BattlerId battler);
static bool32 IsCompactAttackerSlotOccupied(enum BattlerId battler, u8 partyIndex);
static void HandleInputCompactAttackerPicker(enum BattlerId battler);
static void DrawCompactAttackerPicker(enum BattlerId battler);
static void DrawCompactAttackerSummary(enum BattlerId battler);
static void DrawCompactEvolutionIndicator(struct Pokemon *mon, enum Species species);
static void UpdateCompactAttackerStatusSprite(enum BattlerId battler, struct Pokemon *mon);
static void DestroyCompactAttackerStatusSprite(enum BattlerId battler);
static void LoadCompactMoveInfoForAttacker(enum BattlerId battler, struct Pokemon *mon);
static void FormatCompactAttackerName(u8 *dst, struct Pokemon *mon);
static void DrawModernActionMenu(enum BattlerId battler);
#if MODULE_BATTLE_BAG_ENABLED
static void OpenBattleBagMenu(enum BattlerId battler);
static void HandleInputBattleBagMenu(enum BattlerId battler);
static void DrawBattleBagMenu(enum BattlerId battler);
#endif
static void WaitForMonSelection(enum BattlerId battler);
static void CompleteWhenChoseItem(enum BattlerId battler);
static void Task_LaunchLvlUpAnim(u8);
static void Task_PrepareToGiveExpWithExpBar(u8);
static void Task_SetControllerToWaitForString(u8);
static void Task_GiveExpWithExpBar(u8);
static void Task_UpdateLvlInHealthbox(u8);
static void PrintLinkStandbyMsg(void);

static void ReloadMoveNames(enum BattlerId battler);
static u32 CheckTypeEffectiveness(enum BattlerId battlerAtk, enum BattlerId battlerDef);
static u32 CheckTargetTypeEffectiveness(enum BattlerId battler);
static u32 CheckCompactTypeEffectiveness(enum BattlerId battlerAtk, enum BattlerId battlerDef);
static u32 CheckCompactTargetTypeEffectiveness(enum BattlerId battler);
static void MoveSelectionDisplayMoveEffectiveness(u32 foeEffectiveness, enum BattlerId battler);

static void (*const sPlayerBufferCommands[CONTROLLER_CMDS_COUNT])(enum BattlerId battler) =
{
    [CONTROLLER_GETMONDATA]               = BtlController_HandleGetMonData,
    [CONTROLLER_GETRAWMONDATA]            = BtlController_HandleGetRawMonData,
    [CONTROLLER_SETMONDATA]               = BtlController_HandleSetMonData,
    [CONTROLLER_SETRAWMONDATA]            = BtlController_HandleSetRawMonData,
    [CONTROLLER_LOADMONSPRITE]            = PlayerHandleLoadMonSprite,
    [CONTROLLER_SWITCHINANIM]             = BtlController_HandleSwitchInAnim,
    [CONTROLLER_RETURNMONTOBALL]          = BtlController_HandleReturnMonToBall,
    [CONTROLLER_DRAWTRAINERPIC]           = PlayerHandleDrawTrainerPic,
    [CONTROLLER_TRAINERSLIDE]             = PlayerHandleTrainerSlide,
    [CONTROLLER_TRAINERSLIDEBACK]         = PlayerHandleTrainerSlideBack,
    [CONTROLLER_FAINTANIMATION]           = BtlController_HandleFaintAnimation,
    [CONTROLLER_PALETTEFADE]              = PlayerHandlePaletteFade,
    [CONTROLLER_BALLTHROWANIM]            = BtlController_HandleBallThrowAnim,
    [CONTROLLER_PAUSE]                    = PlayerHandlePause,
    [CONTROLLER_MOVEANIMATION]            = BtlController_HandleMoveAnimation,
    [CONTROLLER_PRINTSTRING]              = BtlController_HandlePrintString,
    [CONTROLLER_PRINTSTRINGPLAYERONLY]    = BtlController_HandlePrintStringPlayerOnly,
    [CONTROLLER_CHOOSEACTION]             = PlayerHandleChooseAction,
    [CONTROLLER_YESNOBOX]                 = PlayerHandleYesNoBox,
    [CONTROLLER_CHOOSEMOVE]               = PlayerHandleChooseMove,
    [CONTROLLER_OPENBAG]                  = PlayerHandleChooseItem,
    [CONTROLLER_CHOOSEPOKEMON]            = PlayerHandleChoosePokemon,
    [CONTROLLER_23]                       = PlayerHandleCmd23,
    [CONTROLLER_HEALTHBARUPDATE]          = BtlController_HandleHealthBarUpdate,
    [CONTROLLER_EXPUPDATE]                = PlayerHandleExpUpdate,
    [CONTROLLER_STATUSICONUPDATE]         = BtlController_HandleStatusIconUpdate,
    [CONTROLLER_STATUSANIMATION]          = BtlController_HandleStatusAnimation,
    [CONTROLLER_STATUSXOR]                = PlayerHandleStatusXor,
    [CONTROLLER_DATATRANSFER]             = BtlController_Empty,
    [CONTROLLER_DMA3TRANSFER]             = PlayerHandleDMA3Transfer,
    [CONTROLLER_PLAYBGM]                  = PlayerHandlePlayBGM,
    [CONTROLLER_32]                       = BtlController_Empty,
    [CONTROLLER_TWORETURNVALUES]          = PlayerHandleTwoReturnValues,
    [CONTROLLER_CHOSENMONRETURNVALUE]     = PlayerHandleChosenMonReturnValue,
    [CONTROLLER_ONERETURNVALUE]           = PlayerHandleOneReturnValue,
    [CONTROLLER_ONERETURNVALUE_DUPLICATE] = PlayerHandleOneReturnValue_Duplicate,
    [CONTROLLER_HITANIMATION]             = BtlController_HandleHitAnimation,
    [CONTROLLER_CANTSWITCH]               = BtlController_Empty,
    [CONTROLLER_PLAYSE]                   = BtlController_HandlePlaySE,
    [CONTROLLER_PLAYFANFAREORBGM]         = BtlController_HandlePlayFanfareOrBGM,
    [CONTROLLER_FAINTINGCRY]              = BtlController_HandleFaintingCry,
    [CONTROLLER_INTROSLIDE]               = BtlController_HandleIntroSlide,
    [CONTROLLER_INTROTRAINERBALLTHROW]    = PlayerHandleIntroTrainerBallThrow,
    [CONTROLLER_DRAWPARTYSTATUSSUMMARY]   = PlayerHandleDrawPartyStatusSummary,
    [CONTROLLER_HIDEPARTYSTATUSSUMMARY]   = BtlController_HandleHidePartyStatusSummary,
    [CONTROLLER_ENDBOUNCE]                = PlayerHandleEndBounceEffect,
    [CONTROLLER_SPRITEINVISIBILITY]       = BtlController_HandleSpriteInvisibility,
    [CONTROLLER_BATTLEANIMATION]          = BtlController_HandleBattleAnimation,
    [CONTROLLER_LINKSTANDBYMSG]           = PlayerHandleLinkStandbyMsg,
    [CONTROLLER_RESETACTIONMOVESELECTION] = PlayerHandleResetActionMoveSelection,
    [CONTROLLER_ENDLINKBATTLE]            = PlayerHandleEndLinkBattle,
    [CONTROLLER_DEBUGMENU]                = PlayerHandleBattleDebug,
    [CONTROLLER_TERMINATOR_NOP]           = BtlController_TerminatorNop
};

void SetControllerToPlayer(enum BattlerId battler)
{
    gBattlerBattleController[battler] = BATTLE_CONTROLLER_PLAYER;
    gBattlerControllerEndFuncs[battler] = PlayerBufferExecCompleted;
    gBattlerControllerFuncs[battler] = PlayerBufferRunCommand;
    gDoingBattleAnim = FALSE;
    gPlayerDpadHoldFrames = 0;
}

void PlayerBufferExecCompleted(enum BattlerId battler)
{
    gBattlerControllerFuncs[battler] = PlayerBufferRunCommand;
    if (gBattleTypeFlags & BATTLE_TYPE_LINK)
    {
        u8 playerId = GetMultiplayerId();

        PrepareBufferDataTransferLink(battler, B_COMM_CONTROLLER_IS_DONE, 4, &playerId);
        gBattleResources->bufferA[battler][0] = CONTROLLER_TERMINATOR_NOP;
    }
    else
    {
        MarkBattleControllerIdleOnLocal(battler);
    }
}

static void PlayerBufferRunCommand(enum BattlerId battler)
{
    if (IsBattleControllerActiveOnLocal(battler))
    {
        if (gBattleResources->bufferA[battler][0] < ARRAY_COUNT(sPlayerBufferCommands))
            sPlayerBufferCommands[gBattleResources->bufferA[battler][0]](battler);
        else
            BtlController_Complete(battler);
    }
}

static void CompleteOnBattlerSpritePosX_0(enum BattlerId battler)
{
    if (gSprites[gBattlerSpriteIds[battler]].x2 == 0)
        BtlController_Complete(battler);
}

static enum Item GetPrevBall(enum Item ballId)
{
    s32 i;
    enum PokeBall index = ItemIdToBallId(ballId);
    enum Item newBall = ITEM_NONE;

    for (i = 0; i < POKEBALL_COUNT; i++)
    {
        index--;
        if (index == -1)
            index = POKEBALL_COUNT - 1;
        newBall = gPokeBalls[index].itemId;
        if (CheckBagHasItem(newBall, 1))
            return newBall;
    }
    return ballId;
}

static enum Item GetNextBall(enum Item ballId)
{
    s32 i;
    s32 index = ItemIdToBallId(ballId);
    enum Item newBall = ITEM_NONE;

    for (i = 0; i < POKEBALL_COUNT; i++)
    {
        index++;
        if (index == POKEBALL_COUNT)
            index = 0;
        newBall = gPokeBalls[index].itemId;
        if (CheckBagHasItem(newBall, 1))
            return newBall;
    }
    return ballId;
}

static void HandleInputChooseAction(enum BattlerId battler)
{
    enum Item itemId = gBattleResources->bufferA[battler][2] | (gBattleResources->bufferA[battler][3] << 8);

    DoBounceEffect(battler, BOUNCE_HEALTHBOX, 7, 1);
    DoBounceEffect(battler, BOUNCE_MON, 7, 1);

    if (JOY_REPEAT(DPAD_ANY) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_L_EQUALS_A)
        gPlayerDpadHoldFrames++;
    else
        gPlayerDpadHoldFrames = 0;

    if (B_LAST_USED_BALL == TRUE && B_LAST_USED_BALL_CYCLE == TRUE
    && !(B_LAST_USED_BALL_BUTTON == L_BUTTON && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_L_EQUALS_A))
    {
        if (!gLastUsedBallMenuPresent)
        {
            gBattleStruct->ackBallUseBtn = FALSE;
        }
        else if (JOY_NEW(B_LAST_USED_BALL_BUTTON))
        {
            gBattleStruct->ackBallUseBtn = TRUE;
            gBattleStruct->ballSwapped = FALSE;
            ArrowsChangeColorLastBallCycle(TRUE);
        }

        if (gBattleStruct->ackBallUseBtn)
        {
            if (JOY_HELD(B_LAST_USED_BALL_BUTTON) && (JOY_NEW(DPAD_DOWN) || JOY_NEW(DPAD_RIGHT)))
            {
                bool32 sameBall = FALSE;
                u32 nextBall = GetNextBall(gBallToDisplay);
                gBattleStruct->ballSwapped = TRUE;
                if (gBallToDisplay == nextBall)
                    sameBall = TRUE;
                else
                    gBallToDisplay = nextBall;
                SwapBallToDisplay(sameBall);
                PlaySE(SE_SELECT);
            }
            else if (JOY_HELD(B_LAST_USED_BALL_BUTTON) && (JOY_NEW(DPAD_UP) || JOY_NEW(DPAD_LEFT)))
            {
                bool32 sameBall = FALSE;
                u32 prevBall = GetPrevBall(gBallToDisplay);
                gBattleStruct->ballSwapped = TRUE;
                if (gBallToDisplay == prevBall)
                    sameBall = TRUE;
                else
                    gBallToDisplay = prevBall;
                SwapBallToDisplay(sameBall);
                PlaySE(SE_SELECT);
            }
            else if (JOY_NEW(B_BUTTON) || (!JOY_HELD(B_LAST_USED_BALL_BUTTON) && gBattleStruct->ballSwapped))
            {
                gBattleStruct->ackBallUseBtn = FALSE;
                gBattleStruct->ballSwapped = FALSE;
                ArrowsChangeColorLastBallCycle(FALSE);
            }
            else if (!JOY_HELD(B_LAST_USED_BALL_BUTTON) && CanThrowLastUsedBall())
            {
                gBattleStruct->ackBallUseBtn = FALSE;
                PlaySE(SE_SELECT);
                ArrowsChangeColorLastBallCycle(FALSE);
                TryHideLastUsedBall();
                BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_THROW_BALL, 0);
                BtlController_Complete(battler);
            }
            return;
        }
    }

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        TryHideLastUsedBall();
        sUsingModernActionMenu = FALSE;

        switch (gActionSelectionCursor[battler])
        {
        case 0: // Battle
            if (CanChooseReserveAttacker(battler))
            {
                sBlockCompactAUntilReleased[battler] = TRUE;
                OpenCompactAttackerPicker(battler);
                return;
            }
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_USE_MOVE, 0);
            break;
        case 1: // Bag
            if (Nuzlocke_IsActive()
             && (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
             && !(gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK))
             && (ExtendedOptions_Get(EXT_OPT_NUZLOCKE_BATTLE_ITEMS) == NUZLOCKE_BATTLE_ITEMS_BANNED
              || (ExtendedOptions_Get(EXT_OPT_NUZLOCKE_BATTLE_ITEMS) == NUZLOCKE_BATTLE_ITEMS_ONE
               && gBattleResults.playerItemsUsed != 0)))
            {
                PlaySE(SE_BOO);
                sUsingModernActionMenu = TRUE;
                return;
            }
#if MODULE_BATTLE_BAG_ENABLED
            OpenBattleBagMenu(battler);
            return;
#else
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_USE_ITEM, 0);
#endif
            break;
        case 2: // Pokémon
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_SWITCH, 0);
            break;
        case 3: // Run
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_RUN, 0);
            break;
        }
        BtlController_Complete(battler);
    }
    else if (sUsingModernActionMenu && JOY_NEW(DPAD_UP))
    {
        PlaySE(SE_SELECT);
        gActionSelectionCursor[battler] = (gActionSelectionCursor[battler] + 3) % 4;
        DrawModernActionMenu(battler);
    }
    else if (sUsingModernActionMenu && JOY_NEW(DPAD_DOWN))
    {
        PlaySE(SE_SELECT);
        gActionSelectionCursor[battler] = (gActionSelectionCursor[battler] + 1) % 4;
        DrawModernActionMenu(battler);
    }
    else if (!sUsingModernActionMenu && JOY_NEW(DPAD_LEFT))
    {
        if (gActionSelectionCursor[battler] & 1) // if is B_ACTION_USE_ITEM or B_ACTION_RUN
        {
            PlaySE(SE_SELECT);
            ActionSelectionDestroyCursorAt(gActionSelectionCursor[battler]);
            gActionSelectionCursor[battler] ^= 1;
            ActionSelectionCreateCursorAt(gActionSelectionCursor[battler], 0);
            DrawModernActionMenu(battler);
        }
    }
    else if (!sUsingModernActionMenu && JOY_NEW(DPAD_RIGHT))
    {
        if (!(gActionSelectionCursor[battler] & 1)) // if is B_ACTION_USE_MOVE or B_ACTION_SWITCH
        {
            PlaySE(SE_SELECT);
            ActionSelectionDestroyCursorAt(gActionSelectionCursor[battler]);
            gActionSelectionCursor[battler] ^= 1;
            ActionSelectionCreateCursorAt(gActionSelectionCursor[battler], 0);
            DrawModernActionMenu(battler);
        }
    }
    else if (!sUsingModernActionMenu && JOY_NEW(DPAD_UP))
    {
        if (gActionSelectionCursor[battler] & 2) // if is B_ACTION_SWITCH or B_ACTION_RUN
        {
            PlaySE(SE_SELECT);
            ActionSelectionDestroyCursorAt(gActionSelectionCursor[battler]);
            gActionSelectionCursor[battler] ^= 2;
            ActionSelectionCreateCursorAt(gActionSelectionCursor[battler], 0);
            DrawModernActionMenu(battler);
        }
    }
    else if (!sUsingModernActionMenu && JOY_NEW(DPAD_DOWN))
    {
        if (!(gActionSelectionCursor[battler] & 2)) // if is B_ACTION_USE_MOVE or B_ACTION_USE_ITEM
        {
            PlaySE(SE_SELECT);
            ActionSelectionDestroyCursorAt(gActionSelectionCursor[battler]);
            gActionSelectionCursor[battler] ^= 2;
            ActionSelectionCreateCursorAt(gActionSelectionCursor[battler], 0);
            DrawModernActionMenu(battler);
        }
    }
    else if (JOY_NEW(B_BUTTON) || gPlayerDpadHoldFrames > 59)
    {
        if (IsDoubleBattle()
         && GetBattlerPosition(battler) == B_POSITION_PLAYER_RIGHT
         && !(gAbsentBattlerFlags & (1u << GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)))
         && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
        {
            // Return item to bag if partner had selected one (if consumable).
            if (gBattleResources->bufferA[battler][1] == B_ACTION_USE_ITEM && GetItemConsumability(itemId))
            {
                AddBagItem(itemId, 1);
            }
            PlaySE(SE_SELECT);
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_CANCEL_PARTNER, 0);
            BtlController_Complete(battler);
        }
        else if (B_QUICK_MOVE_CURSOR_TO_RUN)
        {
            if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER)) // If wild battle, pressing B moves cursor to "Run".
            {
                PlaySE(SE_SELECT);
                ActionSelectionDestroyCursorAt(gActionSelectionCursor[battler]);
                gActionSelectionCursor[battler] = 3;
                ActionSelectionCreateCursorAt(gActionSelectionCursor[battler], 0);
                DrawModernActionMenu(battler);
            }
        }
    }
    else if (JOY_NEW(START_BUTTON))
    {
        SwapHpBarsWithHpText();
    }
    else if (DEBUG_BATTLE_MENU == TRUE && JOY_NEW(SELECT_BUTTON))
    {
        BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_DEBUG, 0);
        BtlController_Complete(battler);
    }
    else if (B_LAST_USED_BALL == TRUE && B_LAST_USED_BALL_CYCLE == FALSE
             && JOY_NEW(B_LAST_USED_BALL_BUTTON) && CanThrowLastUsedBall())
    {
        PlaySE(SE_SELECT);
        TryHideLastUsedBall();
        BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_THROW_BALL, 0);
        BtlController_Complete(battler);
    }
}

static void OpenCompactAttackerPicker(enum BattlerId battler)
{
    for (u32 i = 0; i < 4; i++)
        ActionSelectionDestroyCursorAt(i);

    // Compact panels live on the third battle-background page, just like the move screen.
    gBattle_BG0_X = 0;
    gBattle_BG0_Y = DISPLAY_HEIGHT * 2;
    sCompactAttackerPickerFromMoveList[battler] = sUsingCompactMoveList[battler];
    sCompactAttackerCursor[battler] = gBattlerPartyIndexes[battler];
    // Battle sprites are reset between battles, so initialise this picker-owned
    // slot before its first use rather than relying on non-zero EWRAM data.
    sCompactAttackerStatusSpriteIds[battler] = SPRITE_NONE;
    LoadPartyMenuAilmentGfx();
    DrawModernMoveSelectionPanels();
    DrawCompactAttackerPicker(battler);
    gBattlerControllerFuncs[battler] = HandleInputCompactAttackerPicker;
}

static bool32 IsCompactAttackerSlotNavigable(u8 partyIndex)
{
    if (partyIndex >= PARTY_SIZE)
        return FALSE;

    return GetMonData(&gParties[B_TRAINER_PLAYER][partyIndex],
                      MON_DATA_SPECIES_OR_EGG) != SPECIES_NONE;
}

//static bool32 IsCompactAttackerSlotOccupied(u8 partyIndex)
static bool32 IsCompactAttackerSlotOccupied(enum BattlerId battler, u8 partyIndex)
{
    enum BattlerId otherBattler;

    if (partyIndex >= PARTY_SIZE)
        return FALSE;

    if (GetMonData(&gParties[B_TRAINER_PLAYER][partyIndex],
                   MON_DATA_SPECIES_OR_EGG) == SPECIES_NONE)
        return FALSE;

    if (!Nuzlocke_IsMonUsable(&gParties[B_TRAINER_PLAYER][partyIndex]))
        return FALSE;

    for (otherBattler = 0; otherBattler < gBattlersCount; otherBattler++)
    {
        if (otherBattler == battler || !IsOnPlayerSide(otherBattler))
            continue;

        // Do not let both player battlers reserve the same attacker this turn.
        // Use explicit selection-phase ownership: actingPartyIndexes can still
        // contain the partner's value from the previous turn until that
        // battler reaches its own turn-start state.
        if ((gBattleStruct->reserveAttackerTurnCommitted & (1u << otherBattler))
         && gBattleStruct->actingPartyIndexes[otherBattler] == partyIndex)
            return FALSE;

        // A reserve Pokemon committed to a charging, multi-turn, or recharge
        // move must return to the battler that selected it.
        if ((gBattleStruct->reserveAttackerRuntimeValid & (1u << partyIndex))
         && gBattleStruct->reserveAttackerRuntimeOwners[partyIndex] == otherBattler
         && (gBattleStruct->reserveAttackerRuntimeMons[partyIndex].volatiles.multipleTurns
          || gBattleStruct->reserveAttackerRuntimeMons[partyIndex].volatiles.rechargeTimer > 0))
            return FALSE;
    }

    return TRUE;
}
static void HandleInputCompactAttackerPicker(enum BattlerId battler)
{
    u8 *cursor = &sCompactAttackerCursor[battler];

    if (sBlockCompactAUntilReleased[battler])
    {
        if (JOY_HELD(A_BUTTON))
            return;

        sBlockCompactAUntilReleased[battler] = FALSE;
    }

    if (JOY_NEW(A_BUTTON))
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][*cursor];

        if (!IsCompactAttackerSlotOccupied(battler, *cursor)
         || GetMonData(mon, MON_DATA_HP) == 0
         || GetMonData(mon, MON_DATA_SPECIES_OR_EGG) == SPECIES_EGG)
        {
            PlaySE(SE_FAILURE);
            return;
        }

        PlaySE(SE_SELECT);
        DestroyCompactAttackerStatusSprite(battler);

        gBattleStruct->actingPartyIndexes[battler] = *cursor;
        gBattleStruct->reserveAttackerSelectionReady |= 1u << battler;
        gBattleStruct->reserveAttackerTurnCommitted |= 1u << battler;

        gMoveSelectionCursor[battler] = 0;
        gMultiUsePlayerCursor = GetOppositeBattler(battler);

        if (sCompactAttackerPickerFromMoveList[battler])
        {
            // We returned here from the move selector.
            // The engine is already waiting for the selected move, so do not
            // send B_ACTION_USE_MOVE again. Reload the chosen attacker's moves
            // and reopen the existing move-selection command locally.
            LoadCompactMoveInfoForAttacker(battler, mon);
            sBlockCompactAUntilReleased[battler] = TRUE;
            sCompactAttackerPickerFromMoveList[battler] = FALSE;
            PlayerHandleChooseMove(battler);
            return;
        }

        // We entered from the main action menu. Tell the engine that the
        // player selected the Battle action so it can request a move.
        BtlController_EmitTwoReturnValues(
            battler,
            B_COMM_TO_ENGINE,
            B_ACTION_USE_MOVE,
            0
        );
        BtlController_Complete(battler);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        DestroyCompactAttackerStatusSprite(battler);

        gBattleStruct->actingPartyIndexes[battler] =
            gBattlerPartyIndexes[battler];

        gBattleStruct->reserveAttackerSelectionReady &=
            ~(1u << battler);
        gBattleStruct->reserveAttackerTurnCommitted &=
            ~(1u << battler);

        gMoveSelectionCursor[battler] = 0;
        gMultiUsePlayerCursor = GetOppositeBattler(battler);

        if (sCompactAttackerPickerFromMoveList[battler])
        {
            // We arrived from the move selector. Properly cancel the engine's
            // pending choose-move command.
            sCompactAttackerPickerFromMoveList[battler] = FALSE;
            sUsingCompactMoveList[battler] = FALSE;

            BtlController_EmitTwoReturnValues(
                battler,
                B_COMM_TO_ENGINE,
                B_ACTION_EXEC_SCRIPT,
                0xFFFF
            );

            BtlController_Complete(battler);
        }
        else
        {
            // We arrived directly from the main action menu. No engine command
            // needs cancelling; simply redraw that menu.
            PlayerHandleChooseAction(battler);
        }
    }
    else if (JOY_NEW(DPAD_LEFT) && (*cursor & 1) && IsCompactAttackerSlotNavigable(*cursor - 1))
    {
        PlaySE(SE_SELECT);
        (*cursor)--;
        DrawCompactAttackerPicker(battler);
    }
    else if (JOY_NEW(DPAD_RIGHT) && !(*cursor & 1) && IsCompactAttackerSlotNavigable(*cursor + 1))
    {
        PlaySE(SE_SELECT);
        (*cursor)++;
        DrawCompactAttackerPicker(battler);
    }
    else if (JOY_NEW(DPAD_UP) && *cursor >= 2 && IsCompactAttackerSlotNavigable(*cursor - 2))
    {
        PlaySE(SE_SELECT);
        *cursor -= 2;
        DrawCompactAttackerPicker(battler);
    }
    else if (JOY_NEW(DPAD_DOWN) && *cursor < 4 && IsCompactAttackerSlotNavigable(*cursor + 2))
    {
        PlaySE(SE_SELECT);
        *cursor += 2;
        DrawCompactAttackerPicker(battler);
    }
}

static void DrawCompactAttackerPicker(enum BattlerId battler)
{
    static const u8 sCursor[] = _("{RIGHT_ARROW}");
    static const u8 sBlank[] = _(" ");
    static const u8 sEmptySlot[] = _("-");

    // The panel frames are created once when the picker opens. Cursor movement
    // only refreshes their contents, avoiding a visible background flash.
    FillWindowPixelBuffer(B_WIN_MOVE_NAME_1, PIXEL_FILL(0xE));
    FillWindowPixelBuffer(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(0xE));

    DrawCompactAttackerSummary(battler);

    for (u32 i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        // Two 52-pixel cells use the full width of the left compact panel.
        u8 x = (i & 1) ? 52 : 0;
static const u8 sPartyRowY[3] =
{
    0,
    12,
    24,
};

u8 y = sPartyRowY[i / 2];
        const u8 *name = sEmptySlot;

        if (GetMonData(mon, MON_DATA_SPECIES_OR_EGG) != SPECIES_NONE)
        {
            FormatCompactAttackerName(sCompactAttackerNames[i], mon);
            name = sCompactAttackerNames[i];
        }

        AddTextPrinterParameterized4(B_WIN_MOVE_NAME_1, FONT_COMPACT, x, y, 0, 0,
                                     sCompactMoveTextColors, TEXT_SKIP_DRAW, (i == sCompactAttackerCursor[battler]) ? sCursor : sBlank);
        AddTextPrinterParameterized4(B_WIN_MOVE_NAME_1, FONT_COMPACT, x + 8, y, 0, 0,
                                     sCompactMoveTextColors, TEXT_SKIP_DRAW, name);
    }

    PutWindowTilemap(B_WIN_MOVE_NAME_1);
    CopyWindowToVram(B_WIN_MOVE_NAME_1, COPYWIN_FULL);
    PutWindowTilemap(B_WIN_MOVE_DESCRIPTION);
    CopyWindowToVram(B_WIN_MOVE_DESCRIPTION, COPYWIN_FULL);
}

static void DrawCompactAttackerSummary(enum BattlerId battler)
{
    static const u8 sLevel[] = _("Lv");
    static const u8 sHp[] = _(" HP");
    static const u8 sSlash[] = _("/");
    static const u8 sTypeSlash[] = _("/");
    static const u8 sExp[] = _("EXP:");
    static const u8 sExpMax[] = _("EXP:MAX");
    static const u8 sEmpty[] = _("-");
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sCompactAttackerCursor[battler]];
    enum Species species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);
    enum Type type1;
    enum Type type2;
    u8 level;
    u8 *dst;

    if (species == SPECIES_NONE || species == SPECIES_EGG)
    {
        DestroyCompactAttackerStatusSprite(battler);
        AddTextPrinterParameterized4(B_WIN_MOVE_DESCRIPTION, FONT_COMPACT, 0, 8, 0, 0,
                                     sCompactMoveTextColors, TEXT_SKIP_DRAW, sEmpty);
        return;
    }

    level = GetMonData(mon, MON_DATA_LEVEL);
    dst = StringCopy(gDisplayedStringBattle, sLevel);
    dst = ConvertIntToDecimalStringN(dst, level, STR_CONV_MODE_LEFT_ALIGN, 3);
    dst = StringAppend(dst, sHp);
    dst = ConvertIntToDecimalStringN(dst, GetMonData(mon, MON_DATA_HP), STR_CONV_MODE_LEFT_ALIGN, 3);
    dst = StringAppend(dst, sSlash);
    dst = ConvertIntToDecimalStringN(dst, GetMonData(mon, MON_DATA_MAX_HP), STR_CONV_MODE_LEFT_ALIGN, 3);
    *dst = EOS;
    AddTextPrinterParameterized4(B_WIN_MOVE_DESCRIPTION, FONT_COMPACT, 0, 0, 0, 0,
                                 sCompactMoveTextColors, TEXT_SKIP_DRAW, gDisplayedStringBattle);
    UpdateCompactAttackerStatusSprite(battler, mon);

    type1 = GetSpeciesType(species, 0);
    type2 = GetSpeciesType(species, 1);
    dst = StringCopy(gDisplayedStringBattle, gTypesInfo[type1].name);
    if (type2 != type1)
    {
        dst = StringAppend(dst, sTypeSlash);
        dst = StringAppend(dst, gTypesInfo[type2].name);
    }
    *dst = EOS;
    AddTextPrinterParameterized4(B_WIN_MOVE_DESCRIPTION, FONT_COMPACT, 0, 8, 0, 0,
                                 sCompactMoveTextColors, TEXT_SKIP_DRAW, gDisplayedStringBattle);

    if (level == MAX_LEVEL)
    {
        AddTextPrinterParameterized4(B_WIN_MOVE_DESCRIPTION, FONT_COMPACT, 0, 16, 0, 0,
                                     sCompactMoveTextColors, TEXT_SKIP_DRAW, sExpMax);
    }
    else
    {
        u32 exp = GetMonData(mon, MON_DATA_EXP);
        u32 expAtCurrentLevel = gExperienceTables[gSpeciesInfo[species].growthRate][level];
        u32 expAtNextLevel = gExperienceTables[gSpeciesInfo[species].growthRate][level + 1];

        dst = StringCopy(gDisplayedStringBattle, sExp);
        dst = ConvertIntToDecimalStringN(dst, exp - expAtCurrentLevel, STR_CONV_MODE_LEFT_ALIGN, 5);
        dst = StringAppend(dst, sSlash);
        dst = ConvertIntToDecimalStringN(dst, expAtNextLevel - expAtCurrentLevel, STR_CONV_MODE_LEFT_ALIGN, 5);
        *dst = EOS;
        AddTextPrinterParameterized4(B_WIN_MOVE_DESCRIPTION, FONT_COMPACT, 0, 16, 0, 0,
                                     sCompactMoveTextColors, TEXT_SKIP_DRAW, gDisplayedStringBattle);
    }

    DrawCompactEvolutionIndicator(mon, species);
}

static void DrawCompactEvolutionIndicator(struct Pokemon *mon, enum Species species)
{
    static const u8 sEvo[] = _("EVO");
    static const u8 sLv[] = _(" LV");
    static const u8 sReady[] = _("READY");
    static const u8 sItem[] = _("ITEM");
    static const u8 sTrade[] = _("TRADE");
    static const u8 sSpecial[] = _("SPEC");
    static const u8 sFinal[] = _("FINAL");
    const struct Evolution *evolutions = GetSpeciesEvolutions(species);
    const u8 *methodText = sFinal;
    u16 nearestLevel = 0xFFFF;
    u8 level = GetMonData(mon, MON_DATA_LEVEL);
    bool32 hasItemEvolution = FALSE;
    bool32 hasTradeEvolution = FALSE;
    bool32 hasSpecialEvolution = FALSE;
    u8 text[12];
    u8 *dst;

    if (evolutions != NULL)
    {
        for (u32 i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            if (SanitizeSpeciesId(evolutions[i].targetSpecies) == SPECIES_NONE)
                continue;

            switch (evolutions[i].method)
            {
            case EVO_LEVEL:
            case EVO_LEVEL_BATTLE_ONLY:
                if (evolutions[i].param != 0 && evolutions[i].param < nearestLevel)
                    nearestLevel = evolutions[i].param;
                else if (evolutions[i].param == 0)
                    hasSpecialEvolution = TRUE;
                break;
            case EVO_ITEM:
                hasItemEvolution = TRUE;
                break;
            case EVO_TRADE:
                hasTradeEvolution = TRUE;
                break;
            case EVO_NONE:
            case EVO_SPLIT_FROM_EVO:
                break;
            default:
                hasSpecialEvolution = TRUE;
                break;
            }
        }
    }

    AddTextPrinterParameterized4(B_WIN_MOVE_DESCRIPTION, FONT_COMPACT, 66, 8, 0, 0,
                                 sCompactMoveTextColors, TEXT_SKIP_DRAW, sEvo);

    if (nearestLevel != 0xFFFF)
    {
        if (nearestLevel <= level)
        {
            methodText = sReady;
        }
        else
        {
            dst = ConvertIntToDecimalStringN(text, nearestLevel - level, STR_CONV_MODE_LEFT_ALIGN, 2);
            dst = StringAppend(dst, sLv);
            *dst = EOS;
            methodText = text;
        }
    }
    else if (hasItemEvolution)
    {
        methodText = sItem;
    }
    else if (hasTradeEvolution)
    {
        methodText = sTrade;
    }
    else if (hasSpecialEvolution)
    {
        methodText = sSpecial;
    }

    AddTextPrinterParameterized4(B_WIN_MOVE_DESCRIPTION, FONT_COMPACT, 66, 16, 0, 0,
                                 sCompactMoveTextColors, TEXT_SKIP_DRAW, methodText);
}

static void UpdateCompactAttackerStatusSprite(enum BattlerId battler, struct Pokemon *mon)
{
    u8 ailment = GetMonAilment(mon);
    u8 spriteId = sCompactAttackerStatusSpriteIds[battler];

    if (ailment == AILMENT_NONE || ailment == AILMENT_PKRS)
    {
        DestroyCompactAttackerStatusSprite(battler);
        return;
    }

    if (spriteId == SPRITE_NONE)
    {
        // The compact summary occupies x 136-231 and y 120-151 on screen.
        // The stock 32x8 status badge sits neatly against its upper-right edge.
spriteId = CreateSprite(&gSpriteTemplate_StatusIcons, 223, 123, 0);
        if (spriteId == MAX_SPRITES)
            return;

        sCompactAttackerStatusSpriteIds[battler] = spriteId;
        gSprites[spriteId].oam.priority = 0;
    }

    StartSpriteAnim(&gSprites[spriteId], ailment - 1);
    gSprites[spriteId].invisible = FALSE;
}
static void DestroyCompactAttackerStatusSprite(enum BattlerId battler)
{
    u8 spriteId = sCompactAttackerStatusSpriteIds[battler];

    if (spriteId != SPRITE_NONE)
    {
        DestroySprite(&gSprites[spriteId]);
        sCompactAttackerStatusSpriteIds[battler] = SPRITE_NONE;
    }
}
static void DrawModernActionMenu(enum BattlerId battler)
{
    static const u8 sActionMenuY[4] =
    {
        0,
        8,
        16,
        24,
    };

    static const u8 sCursor[] = _("{RIGHT_ARROW}");
    static const u8 sBlank[] = _(" ");
    static const u8 sBattle[] = _("Battle");
    static const u8 sBag[] = _("Bag");
    static const u8 sPokemon[] = _("Pokémon");
    static const u8 sRun[] = _("Run");
    static const u8 *const sActions[] = {sBattle, sBag, sPokemon, sRun};

    static const u8 sEnemyInfo[] = _("Enemy Info:");
    static const u8 sHp[] = _("HP:");
    static const u8 sType[] = _("T:");
    static const u8 sSpace[] = _(" ");
    static const u8 sSlash[] = _("/");
    static const u8 sTypeSlash[] = _("/");
    static const u8 sWeak[] = _("Weak:");
    static const u8 sNone[] = _("--");

    enum BattlerId opponent = GetOppositeBattler(battler);
    enum Type type1 = gBattleMons[opponent].types[0];
    enum Type type2 = gBattleMons[opponent].types[1];
    u8 *dst;

    if (sModernActionMenuPromptNeedsRefresh)
    {
        HandleBattleWindow(0, 34, 29, 39, WINDOW_CLEAR);
        HandleBattleWindow(0, 34, 13, 39, 0);
        HandleBattleWindow(14, 34, 29, 39, 0);
    }

    FillWindowPixelBuffer(B_WIN_ACTION_MENU, PIXEL_FILL(0xE));

  for (u32 i = 0; i < 4; i++)
{
    dst = StringCopy(
        gDisplayedStringBattle,
        i == gActionSelectionCursor[battler] ? sCursor : sBlank
    );

    StringAppend(dst, sActions[i]);

AddTextPrinterParameterized4(
    B_WIN_ACTION_MENU,
    FONT_COMPACT,
    4,
    sActionMenuY[i],
    0,
    0,
    sCompactMoveTextColors,
    TEXT_SKIP_DRAW,
    gDisplayedStringBattle
);
}

    PutWindowTilemap(B_WIN_ACTION_MENU);
    CopyWindowToVram(B_WIN_ACTION_MENU, COPYWIN_FULL);

    if (!sModernActionMenuPromptNeedsRefresh)
        return;

    FillWindowPixelBuffer(B_WIN_ACTION_PROMPT, PIXEL_FILL(0xE));
    AddTextPrinterParameterized4(B_WIN_ACTION_PROMPT, FONT_COMPACT, 0, 0, 0, 0,
                                 sCompactMoveTextColors, TEXT_SKIP_DRAW, sEnemyInfo);

    dst = StringCopy(gDisplayedStringBattle, sHp);
    dst = ConvertIntToDecimalStringN(dst, gBattleMons[opponent].hp, STR_CONV_MODE_LEFT_ALIGN, 3);
    dst = StringAppend(dst, sSlash);
    dst = ConvertIntToDecimalStringN(dst, gBattleMons[opponent].maxHP, STR_CONV_MODE_LEFT_ALIGN, 3);
    dst = StringAppend(dst, sSpace);
    dst = StringAppend(dst, sType);
    dst = StringAppend(dst, sSpace);
    dst = StringAppend(dst, gTypesInfo[type1].name);
    if (type2 != type1)
    {
        dst = StringAppend(dst, sTypeSlash);
        dst = StringAppend(dst, gTypesInfo[type2].name);
    }
    *dst = EOS;
    AddTextPrinterParameterized4(B_WIN_ACTION_PROMPT, FONT_COMPACT, 0, 8, 0, 0,
                                 sCompactMoveTextColors, TEXT_SKIP_DRAW, gDisplayedStringBattle);

    dst = StringCopy(gDisplayedStringBattle, sWeak);
    {
        u16 weaknessWidth = GetStringWidth(FONT_COMPACT, sWeak, 0);
        u8 weaknessCount = 0;

        for (u32 i = 0; i < NUMBER_OF_MON_TYPES; i++)
        {
            enum Type attackType = i;
            uq4_12_t modifier = GetTypeModifier(attackType, type1);
            u16 addedWidth;

            if (type2 != type1)
                modifier = uq4_12_multiply(modifier, GetTypeModifier(attackType, type2));
            if (modifier < UQ_4_12(2.0))
                continue;

            addedWidth = GetStringWidth(FONT_COMPACT, gTypesInfo[attackType].name, 0);
            if (weaknessCount != 0)
                addedWidth += GetStringWidth(FONT_COMPACT, sTypeSlash, 0);
            if (weaknessWidth + addedWidth > WindowWidthPx(B_WIN_ACTION_PROMPT))
                continue;

            if (weaknessCount != 0)
                dst = StringAppend(dst, sTypeSlash);
            dst = StringAppend(dst, gTypesInfo[attackType].name);
            weaknessWidth += addedWidth;
            weaknessCount++;
        }
        if (weaknessCount == 0)
            StringAppend(dst, sNone);
    }
    AddTextPrinterParameterized4(B_WIN_ACTION_PROMPT, FONT_COMPACT, 0, 16, 0, 0,
                                 sCompactMoveTextColors, TEXT_SKIP_DRAW, gDisplayedStringBattle);
    PutWindowTilemap(B_WIN_ACTION_PROMPT);
    CopyWindowToVram(B_WIN_ACTION_PROMPT, COPYWIN_FULL);
    CopyBgTilemapBufferToVram(0);
    sModernActionMenuPromptNeedsRefresh = FALSE;
}

#if MODULE_BATTLE_BAG_ENABLED
enum
{
    BATTLE_BAG_HEALING,
    BATTLE_BAG_BATTLE,
    BATTLE_BAG_BALLS,
    BATTLE_BAG_BERRIES,
    BATTLE_BAG_COUNT,
};

static enum Pocket GetBattleBagPocket(enum BattlerId battler)
{
    static const enum Pocket sPockets[BATTLE_BAG_COUNT] =
    {
        [BATTLE_BAG_HEALING] = POCKET_ITEMS,
        [BATTLE_BAG_BATTLE] = POCKET_ITEMS,
        [BATTLE_BAG_BALLS] = POCKET_POKE_BALLS,
        [BATTLE_BAG_BERRIES] = POCKET_BERRIES,
    };

    return sPockets[sBattleBagCursor[battler]];
}

static u32 GetBattleBagPocketCapacity(enum Pocket pocket)
{
    switch (pocket)
    {
    case POCKET_ITEMS:
        return BAG_ITEMS_COUNT;
    case POCKET_POKE_BALLS:
        return BAG_POKEBALLS_COUNT;
    case POCKET_BERRIES:
        return BAG_BERRIES_COUNT;
    default:
        return 0;
    }
}

static enum Item GetBattleBagItemAt(enum BattlerId battler, u8 pocketPos)
{
    enum Item item = GetBagItemId(GetBattleBagPocket(battler), pocketPos);
    u16 battleUsage;

    if (item == ITEM_NONE || GetItemBattleUsage(item) == 0)
        return ITEM_NONE;

    battleUsage = GetItemBattleUsage(item);
    if (sBattleBagCursor[battler] == BATTLE_BAG_HEALING)
    {
        if (battleUsage != EFFECT_ITEM_RESTORE_HP
         && battleUsage != EFFECT_ITEM_CURE_STATUS
         && battleUsage != EFFECT_ITEM_HEAL_AND_CURE_STATUS
         && battleUsage != EFFECT_ITEM_REVIVE
         && battleUsage != EFFECT_ITEM_RESTORE_PP)
            return ITEM_NONE;
    }
    else if (sBattleBagCursor[battler] == BATTLE_BAG_BATTLE)
    {
        if (battleUsage == EFFECT_ITEM_RESTORE_HP
         || battleUsage == EFFECT_ITEM_CURE_STATUS
         || battleUsage == EFFECT_ITEM_HEAL_AND_CURE_STATUS
         || battleUsage == EFFECT_ITEM_REVIVE
         || battleUsage == EFFECT_ITEM_RESTORE_PP)
            return ITEM_NONE;
    }
    return item;
}

static bool8 SeekBattleBagItem(enum BattlerId battler, s32 direction)
{
    u32 capacity = GetBattleBagPocketCapacity(GetBattleBagPocket(battler));
    s32 position = sBattleBagPocketPos[battler];

    for (u32 i = 0; i < capacity; i++)
    {
        position += direction;
        if (position < 0)
            position = capacity - 1;
        else if ((u32)position >= capacity)
            position = 0;

        if (GetBattleBagItemAt(battler, position) != ITEM_NONE)
        {
            sBattleBagPocketPos[battler] = position;
            return TRUE;
        }
    }
    return FALSE;
}

static u32 GetBattleBagUsableItemCount(enum BattlerId battler)
{
    u32 count = 0;
    u32 capacity = GetBattleBagPocketCapacity(GetBattleBagPocket(battler));

    for (u32 i = 0; i < capacity; i++)
    {
        if (GetBattleBagItemAt(battler, i) != ITEM_NONE)
            count++;
    }
    return count;
}

static u32 GetBattleBagItemOrdinal(enum BattlerId battler, u8 pocketPos)
{
    u32 ordinal = 0;

    for (u32 i = 0; i < pocketPos; i++)
    {
        if (GetBattleBagItemAt(battler, i) != ITEM_NONE)
            ordinal++;
    }
    return ordinal;
}

static u8 GetBattleBagPocketPosFromOrdinal(enum BattlerId battler, u32 ordinal)
{
    u32 capacity = GetBattleBagPocketCapacity(GetBattleBagPocket(battler));

    for (u32 i = 0, current = 0; i < capacity; i++)
    {
        if (GetBattleBagItemAt(battler, i) == ITEM_NONE)
            continue;
        if (current++ == ordinal)
            return i;
    }
    return 0;
}

static void KeepBattleBagSelectionVisible(enum BattlerId battler)
{
    u32 ordinal = GetBattleBagItemOrdinal(battler, sBattleBagPocketPos[battler]);

if (ordinal < sBattleBagListTop[battler])
    sBattleBagListTop[battler] = ordinal;
else if (ordinal >= sBattleBagListTop[battler] + 4)
    sBattleBagListTop[battler] = ordinal - 3;
}

static const u8 *PrepareBattleBagDescription(enum Item item)
{
    u32 lineBreaks = 0;

    StringCopy(gStringVar4, GetItemDescription(item));
    StripLineBreaks(gStringVar4);
    BreakStringAutomatic(gStringVar4, WindowWidthPx(B_WIN_ACTION_PROMPT) - 2, 2,
                         FONT_COMPACT, HIDE_SCROLL_PROMPT);

    for (u32 i = 0; gStringVar4[i] != EOS; i++)
    {
        if (gStringVar4[i] == CHAR_NEWLINE && ++lineBreaks == 2)
        {
            gStringVar4[i] = EOS;
            break;
        }
    }
    return gStringVar4;
}

static bool8 OpenBattleBagPocket(enum BattlerId battler)
{
    sBattleBagPocketPos[battler] = 0;
    sBattleBagListTop[battler] = 0;
    if (GetBattleBagItemAt(battler, 0) == ITEM_NONE && !SeekBattleBagItem(battler, 1))
    {
        sBattleBagPocketEmpty[battler] = TRUE;
        sBattleBagBrowsingItems[battler] = TRUE;
        PlaySE(SE_BOO);
        return TRUE;
    }

    sBattleBagPocketEmpty[battler] = FALSE;
    sBattleBagBrowsingItems[battler] = TRUE;
    return TRUE;
}

static void OpenBattleBagMenu(enum BattlerId battler)
{
    switch (gBagPosition.pocket)
    {
    case POCKET_POKE_BALLS:
        sBattleBagCursor[battler] = BATTLE_BAG_BALLS;
        break;
    case POCKET_BERRIES:
        sBattleBagCursor[battler] = BATTLE_BAG_BERRIES;
        break;
    default:
        if (sBattleBagCursor[battler] != BATTLE_BAG_HEALING
         && sBattleBagCursor[battler] != BATTLE_BAG_BATTLE)
            sBattleBagCursor[battler] = BATTLE_BAG_HEALING;
        break;
    }

    sBattleBagBrowsingItems[battler] = FALSE;
    sBattleBagPocketEmpty[battler] = FALSE;
    sBattleBagBlockAUntilReleased[battler] = TRUE;
    DrawBattleBagMenu(battler);
    gBattlerControllerFuncs[battler] = HandleInputBattleBagMenu;
}

static void HandleInputBattleBagMenu(enum BattlerId battler)
{
    u8 oldCursor = sBattleBagCursor[battler];

    if (sBattleBagBlockAUntilReleased[battler])
    {
        if (!JOY_HELD(A_BUTTON))
            sBattleBagBlockAUntilReleased[battler] = FALSE;
        return;
    }

    if (sBattleBagBrowsingItems[battler])
    {
        if (JOY_NEW(DPAD_UP))
        {
            SeekBattleBagItem(battler, -1);
            KeepBattleBagSelectionVisible(battler);
            PlaySE(SE_SELECT);
            DrawBattleBagMenu(battler);
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            SeekBattleBagItem(battler, 1);
            KeepBattleBagSelectionVisible(battler);
            PlaySE(SE_SELECT);
            DrawBattleBagMenu(battler);
        }
        else if (JOY_NEW(A_BUTTON))
        {
            enum Item item = GetBattleBagItemAt(battler, sBattleBagPocketPos[battler]);
            struct Pokemon *mon = GetBattlerMon(battler);

            if (sBattleBagPocketEmpty[battler])
            {
                PlaySE(SE_BOO);
                return;
            }

            gPartyMenu.slotId = gBattlerPartyIndexes[battler];
            if (item == ITEM_NONE || CannotUseItemsInBattle(item, mon))
            {
                PlaySE(SE_BOO);
                return;
            }

            PlaySE(SE_SELECT);
            gBagPosition.pocket = GetBattleBagPocket(battler);
            gSpecialVar_ItemId = item;
            gBattleStruct->itemPartyIndex[battler] = gBattlerPartyIndexes[battler];
            sBattleBagPendingItem[battler] = item;
            sBattleBagItemPending[battler] = TRUE;
            sUsingModernActionMenu = FALSE;
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_USE_ITEM, 0);
            BtlController_Complete(battler);
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            sBattleBagBrowsingItems[battler] = FALSE;
            DrawBattleBagMenu(battler);
        }
        return;
    }

    if (JOY_NEW(DPAD_UP))
    {
        if (sBattleBagCursor[battler] == 0)
            sBattleBagCursor[battler] = BATTLE_BAG_COUNT - 1;
        else
            sBattleBagCursor[battler]--;
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (++sBattleBagCursor[battler] == BATTLE_BAG_COUNT)
            sBattleBagCursor[battler] = 0;
    }
    else if (JOY_NEW(A_BUTTON))
    {
        if (OpenBattleBagPocket(battler))
        {
            PlaySE(SE_SELECT);
            DrawBattleBagMenu(battler);
        }
        return;
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        TryRestoreLastUsedBall();
        sUsingModernActionMenu = TRUE;
        sModernActionMenuPromptNeedsRefresh = TRUE;
        DrawModernActionMenu(battler);
        gBattlerControllerFuncs[battler] = HandleInputChooseAction;
        return;
    }

    if (oldCursor != sBattleBagCursor[battler])
    {
        PlaySE(SE_SELECT);
        DrawBattleBagMenu(battler);
    }
}

static void DrawBattleBagMenu(enum BattlerId battler)
{
    static const u8 sCursor[] = _("{RIGHT_ARROW}");
    static const u8 sBlank[] = _(" ");
    static const u8 sHealing[] = _("HEALING");
    static const u8 sBattle[] = _("BATTLE");
    static const u8 sBalls[] = _("BALLS");
    static const u8 sBerries[] = _("BERRIES");
    static const u8 sTitle[] = _("BATTLE BAG");
    static const u8 sHint[] = _("A:SELECT  B:BACK");
    static const u8 sHealingInfo[] = _("HEALING ITEMS");
    static const u8 sBattleInfo[] = _("BATTLE ITEMS");
    static const u8 sBallsInfo[] = _("POKE BALLS");
    static const u8 sBerriesInfo[] = _("BATTLE BERRIES");
    static const u8 sQuantitySeparator[] = _(" ×");
    static const u8 sNoUsableItems[] = _("NO USABLE ITEMS");
    static const u8 *const sLabels[BATTLE_BAG_COUNT] =
    {
        [BATTLE_BAG_HEALING] = sHealing,
        [BATTLE_BAG_BATTLE] = sBattle,
        [BATTLE_BAG_BALLS] = sBalls,
        [BATTLE_BAG_BERRIES] = sBerries,
    };
    static const u8 *const sPocketInfo[BATTLE_BAG_COUNT] =
    {
        [BATTLE_BAG_HEALING] = sHealingInfo,
        [BATTLE_BAG_BATTLE] = sBattleInfo,
        [BATTLE_BAG_BALLS] = sBallsInfo,
        [BATTLE_BAG_BERRIES] = sBerriesInfo,
    };
    u8 *dst;

    HandleBattleWindow(0, 34, 29, 39, WINDOW_CLEAR);
    HandleBattleWindow(0, 34, 13, 39, 0);
    HandleBattleWindow(14, 34, 29, 39, 0);

    FillWindowPixelBuffer(B_WIN_ACTION_MENU, PIXEL_FILL(0xE));
    FillWindowPixelBuffer(B_WIN_ACTION_PROMPT, PIXEL_FILL(0xE));

    if (sBattleBagBrowsingItems[battler])
    {
        if (sBattleBagPocketEmpty[battler])
        {
            AddTextPrinterParameterized4(B_WIN_ACTION_MENU, FONT_COMPACT, 8, 8, 0, 0,
                                         sCompactMoveTextColors, TEXT_SKIP_DRAW, sNoUsableItems);
        }
        else
        {
            enum Item item = GetBattleBagItemAt(battler, sBattleBagPocketPos[battler]);
            struct ItemSlot slot = GetBagItemIdAndQuantity(GetBattleBagPocket(battler), sBattleBagPocketPos[battler]);
            u32 itemCount = GetBattleBagUsableItemCount(battler);
            u32 selectedOrdinal = GetBattleBagItemOrdinal(battler, sBattleBagPocketPos[battler]);

for (u32 row = 0; row < 4 && sBattleBagListTop[battler] + row < itemCount; row++)
            {
                u32 ordinal = sBattleBagListTop[battler] + row;
                u8 pocketPos = GetBattleBagPocketPosFromOrdinal(battler, ordinal);
                enum Item rowItem = GetBattleBagItemAt(battler, pocketPos);

                dst = StringCopy(gDisplayedStringBattle, ordinal == selectedOrdinal ? sCursor : sBlank);
                CopyItemName(rowItem, gStringVar1);
                StringAppend(dst, gStringVar1);
                AddTextPrinterParameterized4(B_WIN_ACTION_MENU, FONT_COMPACT, 0, row * 8, 0, 0,
                                             sCompactMoveTextColors, TEXT_SKIP_DRAW, gDisplayedStringBattle);
            }

            ConvertIntToDecimalStringN(gStringVar2, slot.quantity, STR_CONV_MODE_LEFT_ALIGN, 3);
            dst = StringCopy(gDisplayedStringBattle, sQuantitySeparator);
            StringAppend(dst, gStringVar2);
            AddTextPrinterParameterized4(B_WIN_ACTION_PROMPT, FONT_COMPACT,
                                         WindowWidthPx(B_WIN_ACTION_PROMPT)
                                         - GetStringWidth(FONT_COMPACT, gDisplayedStringBattle, 0),
                                         0, 0, 0,
                                         sCompactMoveTextColors, TEXT_SKIP_DRAW, gDisplayedStringBattle);
            AddTextPrinterParameterized4(B_WIN_ACTION_PROMPT, FONT_COMPACT, 0, 8, 0, 0,
                                         sCompactMoveTextColors, TEXT_SKIP_DRAW, PrepareBattleBagDescription(item));
        }
    }
    else
    {
        AddTextPrinterParameterized4(B_WIN_ACTION_PROMPT, FONT_COMPACT, 0, 0, 0, 0,
                                     sCompactMoveTextColors, TEXT_SKIP_DRAW, sTitle);
        AddTextPrinterParameterized4(B_WIN_ACTION_PROMPT, FONT_COMPACT, 0, 8, 0, 0,
                                     sCompactMoveTextColors, TEXT_SKIP_DRAW,
                                     sPocketInfo[sBattleBagCursor[battler]]);
        AddTextPrinterParameterized4(B_WIN_ACTION_PROMPT, FONT_COMPACT, 0, 16, 0, 0,
                                     sCompactMoveTextColors, TEXT_SKIP_DRAW, sHint);

        // Pocket/category selector uses the same fixed four-row layout as
        // the main action menu. The actual item browser below remains a
        // scrolling three-row list.
        for (u32 pocket = 0; pocket < BATTLE_BAG_COUNT; pocket++)
        {
            dst = StringCopy(gDisplayedStringBattle,
                             pocket == sBattleBagCursor[battler] ? sCursor : sBlank);
            StringAppend(dst, sLabels[pocket]);
            AddTextPrinterParameterized4(B_WIN_ACTION_MENU, FONT_COMPACT, 4, pocket * 8, 0, 0,
                                         sCompactMoveTextColors, TEXT_SKIP_DRAW, gDisplayedStringBattle);
        }
    }

    // FONT_COMPACT is a true 8px renderer, so no post-draw pixel scroll is
    // needed. Keeping coordinates on the 8px grid avoids cross-page offsets.
    PutWindowTilemap(B_WIN_ACTION_MENU);
    CopyWindowToVram(B_WIN_ACTION_MENU, COPYWIN_FULL);
    PutWindowTilemap(B_WIN_ACTION_PROMPT);
    CopyWindowToVram(B_WIN_ACTION_PROMPT, COPYWIN_FULL);
    CopyBgTilemapBufferToVram(0);
}
#endif // MODULE_BATTLE_BAG_ENABLED

void DrawModernActionMenuForScriptedBattle(enum BattlerId battler)
{
    sUsingCompactMoveList[battler] = FALSE;
    sUsingModernActionMenu = TRUE;
    sModernActionMenuPromptNeedsRefresh = TRUE;
    DrawModernActionMenu(battler);
}

static void LoadCompactMoveInfoForAttacker(enum BattlerId battler, struct Pokemon *mon)
{
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    u8 ppBonuses = GetMonData(mon, MON_DATA_PP_BONUSES);

    moveInfo->species = species;
    moveInfo->monTypes[0] = GetSpeciesType(species, 0);
    moveInfo->monTypes[1] = GetSpeciesType(species, 1);
    moveInfo->monTypes[2] = GetSpeciesType(species, 2);

    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        moveInfo->moves[i] = GetMonData(mon, MON_DATA_MOVE1 + i);
        moveInfo->currentPP[i] = GetMonData(mon, MON_DATA_PP1 + i);
        moveInfo->maxPP[i] = CalculatePPWithBonus(moveInfo->moves[i], ppBonuses, i);
    }
}

static void FormatCompactAttackerName(u8 *dst, struct Pokemon *mon)
{
    static const u8 sEllipsis[] = _("…");
    u16 length;

    GetMonNickname(mon, dst);
    length = StringLength(dst);

    // Each column has 44 pixels after its cursor. Keep the name readable and
    // use an ellipsis for genuine overflow instead of silently chopping it.
    while (GetStringWidth(FONT_COMPACT, dst, 0) > 44 && length > 1)
    {
        dst[--length] = EOS;
        StringAppend(dst, sEllipsis);
    }
}

void HandleInputChooseTarget(enum BattlerId battler)
{
    enum BattlerId i;
    static const enum BattlerPosition identities[MAX_BATTLERS_COUNT] =
    {
        B_POSITION_PLAYER_LEFT,
        B_POSITION_PLAYER_RIGHT,
        B_POSITION_OPPONENT_RIGHT,
        B_POSITION_OPPONENT_LEFT,
    };
enum Move move;

if (gBattleStruct->reserveAttackerSelectionReady & (1u << battler))
{
    struct ChooseMoveStruct *moveInfo =
        (struct ChooseMoveStruct *)&gBattleResources->bufferA[battler][4];

    move = moveInfo->moves[gMoveSelectionCursor[battler]];
}
else
{
    move = GetMonData(
        GetBattlerMon(battler),
        MON_DATA_MOVE1 + gMoveSelectionCursor[battler]
    );
}    enum MoveTarget moveTarget = GetBattlerMoveTargetType(battler, move);

    DoBounceEffect(gMultiUsePlayerCursor, BOUNCE_HEALTHBOX, 15, 1);
    for (i = 0; i < gBattlersCount; i++)
    {
        if (i != gMultiUsePlayerCursor)
            EndBounceEffect(i, BOUNCE_HEALTHBOX);
    }

    if (JOY_HELD(DPAD_ANY) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_L_EQUALS_A)
        gPlayerDpadHoldFrames++;
    else
        gPlayerDpadHoldFrames = 0;

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCB_HideAsMoveTarget;
        if (gBattleStruct->gimmick.playerSelect)
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, gMoveSelectionCursor[battler] | RET_GIMMICK | (gMultiUsePlayerCursor << 8));
        else
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, gMoveSelectionCursor[battler] | (gMultiUsePlayerCursor << 8));
        EndBounceEffect(gMultiUsePlayerCursor, BOUNCE_HEALTHBOX);
        TryHideLastUsedBall();
        HideGimmickTriggerSprite();
        BtlController_Complete(battler);
    }
    else if (JOY_NEW(B_BUTTON) || gPlayerDpadHoldFrames > 59)
    {
        PlaySE(SE_SELECT);
        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCB_HideAsMoveTarget;
        gBattlerControllerFuncs[battler] = HandleInputChooseMove;
        if (gBattleStruct->gimmick.playerSelect == 1 && gBattleStruct->gimmick.usableGimmick[battler] == GIMMICK_Z_MOVE)
        {
            gBattleStruct->gimmick.playerSelect = 0;
            gBattleStruct->zmove.viewing = TRUE;
            ReloadMoveNames(battler);
        }
        // The compact move panel displays this information continuously.
        DoBounceEffect(battler, BOUNCE_HEALTHBOX, 7, 1);
        DoBounceEffect(battler, BOUNCE_MON, 7, 1);
        EndBounceEffect(gMultiUsePlayerCursor, BOUNCE_HEALTHBOX);
    }
    else if (JOY_NEW(DPAD_LEFT | DPAD_UP))
    {
        PlaySE(SE_SELECT);
        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCB_HideAsMoveTarget;

        if (moveTarget == TARGET_USER_OR_ALLY)
        {
            gMultiUsePlayerCursor ^= BIT_FLANK;
        }
        else
        {
            bool32 validTarget = FALSE;
            do
            {
                enum BattlerPosition currSelIdentity = GetBattlerPosition(gMultiUsePlayerCursor);

                for (i = 0; i < MAX_BATTLERS_COUNT; i++)
                {
                    if (currSelIdentity == identities[i])
                        break;
                }
                do
                {
                    if (i == 0)
                        i = MAX_BATTLERS_COUNT - 1;
                    else
                        i--;
                    gMultiUsePlayerCursor = GetBattlerAtPosition(identities[i]);
                } while (gMultiUsePlayerCursor >= gBattlersCount);

                switch (GetBattlerPosition(gMultiUsePlayerCursor))
                {
                case B_POSITION_PLAYER_LEFT:
                case B_POSITION_PLAYER_RIGHT:
                    if (battler != gMultiUsePlayerCursor)
                        validTarget = TRUE;
                    break;
                case B_POSITION_OPPONENT_LEFT:
                case B_POSITION_OPPONENT_RIGHT:
                    validTarget = TRUE;
                    break;
                default:
                    break;
                }

                if (!CanTargetBattler(battler, gMultiUsePlayerCursor, move)
                 || (moveTarget == TARGET_OPPONENT && IsOnPlayerSide(gMultiUsePlayerCursor)))
                    validTarget = FALSE;

                if (B_SHOW_EFFECTIVENESS && validTarget)
                    MoveSelectionDisplayMoveEffectiveness(CheckTypeEffectiveness(battler, gMultiUsePlayerCursor), battler);

            } while (!validTarget);
        }
        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCB_ShowAsMoveTarget;
    }
    else if (JOY_NEW(DPAD_RIGHT | DPAD_DOWN))
    {
        PlaySE(SE_SELECT);
        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCB_HideAsMoveTarget;

        if (moveTarget == TARGET_USER_OR_ALLY)
        {
            gMultiUsePlayerCursor ^= BIT_FLANK;
        }
        else
        {
            do
            {
                enum BattlerPosition currSelIdentity = GetBattlerPosition(gMultiUsePlayerCursor);

                for (i = 0; i < MAX_BATTLERS_COUNT; i++)
                {
                    if (currSelIdentity == identities[i])
                        break;
                }
                do
                {
                    if (++i > 3)
                        i = 0;
                    gMultiUsePlayerCursor = GetBattlerAtPosition(identities[i]);
                } while (gMultiUsePlayerCursor == gBattlersCount);

                i = 0;
                switch (GetBattlerPosition(gMultiUsePlayerCursor))
                {
                case B_POSITION_PLAYER_LEFT:
                case B_POSITION_PLAYER_RIGHT:
                    if (battler != gMultiUsePlayerCursor)
                        i++;
                    break;
                case B_POSITION_OPPONENT_LEFT:
                case B_POSITION_OPPONENT_RIGHT:
                    i++;
                    break;
                default:
                    break;
                }
                if (B_SHOW_EFFECTIVENESS)
                    MoveSelectionDisplayMoveEffectiveness(CheckTypeEffectiveness(battler, gMultiUsePlayerCursor), battler);

                if (!CanTargetBattler(battler, gMultiUsePlayerCursor, move)
                 || (moveTarget == TARGET_OPPONENT && IsOnPlayerSide(gMultiUsePlayerCursor)))
                    i = 0;
            } while (i == 0);
        }

        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCB_ShowAsMoveTarget;
    }
}

static void HideAllTargets(void)
{
    for (enum BattlerId i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        if (IsBattlerAlive(i) && gBattleSpritesDataPtr->healthBoxesData[i].healthboxIsBouncing)
        {
            gSprites[gBattlerSpriteIds[i]].callback = SpriteCB_HideAsMoveTarget;
            EndBounceEffect(i, BOUNCE_HEALTHBOX);
        }
    }
}

void HandleInputShowEntireFieldTargets(enum BattlerId battler)
{
    if (JOY_HELD(DPAD_ANY) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_L_EQUALS_A)
        gPlayerDpadHoldFrames++;
    else
        gPlayerDpadHoldFrames = 0;

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        HideAllTargets();
        if (gBattleStruct->gimmick.playerSelect)
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, gMoveSelectionCursor[battler] | RET_GIMMICK | (gMultiUsePlayerCursor << 8));
        else
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, gMoveSelectionCursor[battler] | (gMultiUsePlayerCursor << 8));
        HideGimmickTriggerSprite();
        BtlController_Complete(battler);
    }
    else if (JOY_NEW(B_BUTTON) || gPlayerDpadHoldFrames > 59)
    {
        PlaySE(SE_SELECT);
        HideAllTargets();
        gBattlerControllerFuncs[battler] = HandleInputChooseMove;
        DoBounceEffect(battler, BOUNCE_HEALTHBOX, 7, 1);
        DoBounceEffect(battler, BOUNCE_MON, 7, 1);
    }
}

void HandleInputShowTargets(enum BattlerId battler)
{
    if (JOY_HELD(DPAD_ANY) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_L_EQUALS_A)
        gPlayerDpadHoldFrames++;
    else
        gPlayerDpadHoldFrames = 0;

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        HideAllTargets();
        if (gBattleStruct->gimmick.playerSelect)
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, gMoveSelectionCursor[battler] | RET_GIMMICK | (gMultiUsePlayerCursor << 8));
        else
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, gMoveSelectionCursor[battler] | (gMultiUsePlayerCursor << 8));
        HideGimmickTriggerSprite();
        TryHideLastUsedBall();
        BtlController_Complete(battler);
    }
    else if (JOY_NEW(B_BUTTON) || gPlayerDpadHoldFrames > 59)
    {
        PlaySE(SE_SELECT);
        HideAllTargets();
        gBattlerControllerFuncs[battler] = HandleInputChooseMove;
        DoBounceEffect(battler, BOUNCE_HEALTHBOX, 7, 1);
        DoBounceEffect(battler, BOUNCE_MON, 7, 1);
    }
}

static void TryShowAsTarget(enum BattlerId battler)
{
    if (IsBattlerAlive(battler))
    {
        DoBounceEffect(battler, BOUNCE_HEALTHBOX, 15, 1);
        gSprites[gBattlerSpriteIds[battler]].callback = SpriteCB_ShowAsMoveTarget;
    }
}

static bool32 CanSelectBattler(enum MoveTarget target)
{
    switch (target)
    {
    case TARGET_RANDOM:
    case TARGET_BOTH:
    case TARGET_DEPENDS:
    case TARGET_FOES_AND_ALLY:
    case TARGET_OPPONENTS_FIELD:
    case TARGET_USER:
    case TARGET_ALLY:
    case TARGET_USER_OR_ALLY:
    case TARGET_USER_AND_ALLY:
        return TRUE;
    default:
        break;
    }

    return FALSE;
}

void HandleInputChooseMove(enum BattlerId battler)
{
    u32 canSelectTarget = 0;
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);

    if (sBlockCompactAUntilReleased[battler])
    {
        if (JOY_HELD(A_BUTTON))
            return;

        sBlockCompactAUntilReleased[battler] = FALSE;
    }

    if (JOY_HELD(DPAD_ANY) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_L_EQUALS_A)
        gPlayerDpadHoldFrames++;
    else
        gPlayerDpadHoldFrames = 0;

    if (JOY_NEW(A_BUTTON) && !gBattleStruct->descriptionSubmenu)
    {
        TryToHideMoveInfoWindow();
        PlaySE(SE_SELECT);

        enum MoveTarget moveTarget = GetBattlerMoveTargetType(battler, moveInfo->moves[gMoveSelectionCursor[battler]]);
        bool32 isUserOrAlly = moveTarget == TARGET_USER || moveTarget == TARGET_USER_OR_ALLY || moveTarget == TARGET_USER_AND_ALLY;

        if (gBattleStruct->zmove.viewing)
        {
            gBattleStruct->zmove.viewing = FALSE;
            if (GetMoveCategory(moveInfo->moves[gMoveSelectionCursor[battler]]) != DAMAGE_CATEGORY_STATUS)
                moveTarget = TARGET_SELECTED;  //damaging z moves always have selected target
        }

        // Status moves turn into Max Guard when Dynamaxed, targets user.
        if (GetActiveGimmick(battler) == GIMMICK_DYNAMAX || IsGimmickSelected(battler, GIMMICK_DYNAMAX))
            moveTarget = GetMoveTarget(GetMaxMove(battler, moveInfo->moves[gMoveSelectionCursor[battler]]));

        if (isUserOrAlly)
            gMultiUsePlayerCursor = battler;
        else if (moveTarget == TARGET_ALLY)
            gMultiUsePlayerCursor = GetPartnerBattler(battler);
        else
            gMultiUsePlayerCursor = GetBattlerLeftFoe(battler);

        if (gBattleResources->bufferA[battler][1]) // a double battle
        {
            if (!CanSelectBattler(moveTarget))
                canSelectTarget = 1; // either selected or user
            if (moveTarget == TARGET_USER_OR_ALLY && IsBattlerAlive(GetPartnerBattler(battler)))
                canSelectTarget = 1;

            if (moveInfo->currentPP[gMoveSelectionCursor[battler]] == 0)
            {
                canSelectTarget = 0;
            }
            else if (isUserOrAlly && CountAliveMonsInBattle(BATTLE_ALIVE_EXCEPT_BATTLER, battler) <= 1)
            {
                gMultiUsePlayerCursor = GetDefaultMoveTarget(battler);
                canSelectTarget = 0;
            }

            if (B_SHOW_TARGETS == TRUE)
            {
                // Show all available targets for multi-target moves
                if (moveTarget == TARGET_ALL_BATTLERS || moveTarget == TARGET_FIELD)
                {
                    for (enum BattlerId i = 0; i < gBattlersCount; i++)
                        TryShowAsTarget(i);

                    canSelectTarget = 3;
                }
                else if (IsSpreadMove(moveTarget) || moveTarget == TARGET_OPPONENTS_FIELD || moveTarget == TARGET_USER_AND_ALLY)
                {
                    TryShowAsTarget(gMultiUsePlayerCursor);
                    TryShowAsTarget(GetPartnerBattler(gMultiUsePlayerCursor));
                    if (moveTarget == TARGET_FOES_AND_ALLY)
                        TryShowAsTarget(GetPartnerBattler(battler));
                    canSelectTarget = 2;
                }
            }
        }

        switch (canSelectTarget)
        {
        case 0:
        default:
            if (gBattleStruct->gimmick.playerSelect)
                BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, gMoveSelectionCursor[battler] | RET_GIMMICK | (gMultiUsePlayerCursor << 8));
            else
                BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, gMoveSelectionCursor[battler] | (gMultiUsePlayerCursor << 8));
            HideGimmickTriggerSprite();
            TryHideLastUsedBall();
            BtlController_Complete(battler);
            break;
        case 1:
            gBattlerControllerFuncs[battler] = HandleInputChooseTarget;

            if (moveTarget == TARGET_USER || moveTarget == TARGET_USER_OR_ALLY)
                gMultiUsePlayerCursor = battler;
            else if (gAbsentBattlerFlags & (1u << GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT)))
                gMultiUsePlayerCursor = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
            else
                gMultiUsePlayerCursor = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
            if (B_SHOW_EFFECTIVENESS)
                MoveSelectionDisplayMoveEffectiveness(CheckTypeEffectiveness(battler, gMultiUsePlayerCursor), battler);

            gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCB_ShowAsMoveTarget;
            break;
        case 2:
            gBattlerControllerFuncs[battler] = HandleInputShowTargets;
            break;
        case 3: // Entire field
            gBattlerControllerFuncs[battler] = HandleInputShowEntireFieldTargets;
            break;
        }
    }
    else if ((JOY_NEW(B_BUTTON) || gPlayerDpadHoldFrames > 59)  && !gBattleStruct->descriptionSubmenu)
    {
        PlaySE(SE_SELECT);
        gBattleStruct->gimmick.playerSelect = FALSE;
if (sUsingCompactMoveList[battler])
{
    gBattleStruct->zmove.viewing = FALSE;

    gMoveSelectionCursor[battler] = 0;
    gMultiUsePlayerCursor = GetOppositeBattler(battler);
    sBlockCompactAUntilReleased[battler] = TRUE;

    HideGimmickTriggerSprite();
    TryToHideMoveInfoWindow();
    OpenCompactAttackerPicker(battler);
    return;
}

        if (gBattleStruct->zmove.viewing)
        {
            ReloadMoveNames(battler);
            ChangeGimmickTriggerSprite(gBattleStruct->gimmick.triggerSpriteId, gBattleStruct->gimmick.playerSelect);
        }
        else
        {
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, 0xFFFF);
            HideGimmickTriggerSprite();
            BtlController_Complete(battler);
            TryToHideMoveInfoWindow();
        }
    }
    else if (FALSE && JOY_NEW(DPAD_LEFT) && !gBattleStruct->zmove.viewing)
    {
        if (gMoveSelectionCursor[battler] & 1)
        {
            gMoveSelectionCursor[battler] ^= 1;
            PlaySE(SE_SELECT);
            DrawCompactMoveList(battler);
            if (B_SHOW_EFFECTIVENESS)
                MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
            MoveSelectionDisplayPPNumber(battler);
            MoveSelectionDisplayMoveType(battler);
            TryMoveSelectionDisplayMoveDescription(battler);
            TryChangeZTrigger(battler, gMoveSelectionCursor[battler]);
        }
    }
    else if (FALSE && JOY_NEW(DPAD_RIGHT) && !gBattleStruct->zmove.viewing)
    {
        if (!(gMoveSelectionCursor[battler] & 1)
         && (gMoveSelectionCursor[battler] ^ 1) < gNumberOfMovesToChoose)
        {
            gMoveSelectionCursor[battler] ^= 1;
            PlaySE(SE_SELECT);
            DrawCompactMoveList(battler);
            if (B_SHOW_EFFECTIVENESS)
                MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
            MoveSelectionDisplayPPNumber(battler);
            MoveSelectionDisplayMoveType(battler);
            TryMoveSelectionDisplayMoveDescription(battler);
            TryChangeZTrigger(battler, gMoveSelectionCursor[battler]);
        }
    }
    else if (JOY_NEW(DPAD_UP) && !gBattleStruct->zmove.viewing)
    {
        if (gMoveSelectionCursor[battler] > 0)
        {
            gMoveSelectionCursor[battler]--;
            PlaySE(SE_SELECT);
            DrawCompactMoveList(battler);
            if (B_SHOW_EFFECTIVENESS)
                MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
            MoveSelectionDisplayPPNumber(battler);
            MoveSelectionDisplayMoveType(battler);
            TryMoveSelectionDisplayMoveDescription(battler);
            TryChangeZTrigger(battler, gMoveSelectionCursor[battler]);
        }
    }
    else if (JOY_NEW(DPAD_DOWN) && !gBattleStruct->zmove.viewing)
    {
        if (gMoveSelectionCursor[battler] + 1 < gNumberOfMovesToChoose)
        {
            gMoveSelectionCursor[battler]++;
            PlaySE(SE_SELECT);
            DrawCompactMoveList(battler);
            if (B_SHOW_EFFECTIVENESS)
                MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
            MoveSelectionDisplayPPNumber(battler);
            MoveSelectionDisplayMoveType(battler);
            TryMoveSelectionDisplayMoveDescription(battler);
            TryChangeZTrigger(battler, gMoveSelectionCursor[battler]);
        }
    }
    else if (B_MOVE_REARRANGEMENT_IN_BATTLE < GEN_4 && JOY_NEW(SELECT_BUTTON) && !gBattleStruct->zmove.viewing && !gBattleStruct->descriptionSubmenu)
    {
        if (gNumberOfMovesToChoose > 1 && !(gBattleTypeFlags & BATTLE_TYPE_LINK))
        {
            if (gMoveSelectionCursor[battler] != 0)
                gMultiUsePlayerCursor = 0;
            else
                gMultiUsePlayerCursor = gMoveSelectionCursor[battler] + 1;

            DrawCompactMoveList(battler);
            BattlePutTextOnWindow(gText_BattleSwitchWhich, B_WIN_SWITCH_PROMPT);
            gBattlerControllerFuncs[battler] = HandleMoveSwitching;
        }
    }
    else if (FALSE && gBattleStruct->descriptionSubmenu)
    {
        if (JOY_NEW(B_MOVE_DESCRIPTION_BUTTON) || JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON))
        {
            gBattleStruct->descriptionSubmenu = FALSE;
            if (gCategoryIconSpriteId != 0xFF)
            {
                DestroySprite(&gSprites[gCategoryIconSpriteId]);
                gCategoryIconSpriteId = 0xFF;
            }

            FillWindowPixelBuffer(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(0));
            ClearStdWindowAndFrame(B_WIN_MOVE_DESCRIPTION, FALSE);
            CopyWindowToVram(B_WIN_MOVE_DESCRIPTION, COPYWIN_GFX);
            PlaySE(SE_SELECT);
            if (B_SHOW_EFFECTIVENESS)
                MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
            MoveSelectionDisplayPPNumber(battler);
            MoveSelectionDisplayMoveType(battler);
        }
    }
    else if (FALSE && JOY_NEW(B_MOVE_DESCRIPTION_BUTTON) &&
        !(B_MOVE_DESCRIPTION_BUTTON == L_BUTTON && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_L_EQUALS_A))
    {
        gBattleStruct->descriptionSubmenu = TRUE;
        TryMoveSelectionDisplayMoveDescription(battler);
    }
    else if (JOY_NEW(START_BUTTON))
    {
        if (gBattleStruct->gimmick.usableGimmick[battler] != GIMMICK_NONE
            && !HasTrainerUsedGimmick(battler, gBattleStruct->gimmick.usableGimmick[battler])
            && !(gBattleStruct->gimmick.usableGimmick[battler] == GIMMICK_Z_MOVE
                 && GetUsableZMove(battler, moveInfo->moves[gMoveSelectionCursor[battler]]) == MOVE_NONE))
        {
            gBattleStruct->gimmick.playerSelect ^= 1;
            ReloadMoveNames(battler);
            ChangeGimmickTriggerSprite(gBattleStruct->gimmick.triggerSpriteId, gBattleStruct->gimmick.playerSelect);
            PlaySE(SE_SELECT);
        }
    }
}

static void ReloadMoveNames(enum BattlerId battler)
{
    if (gBattleStruct->zmove.viable && !gBattleStruct->zmove.viewing)
    {
        sUsingCompactMoveList[battler] = FALSE;
        struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
        MoveSelectionDisplayZMove(GetUsableZMove(battler, moveInfo->moves[gMoveSelectionCursor[battler]]), battler);
    }
    else
    {
        gBattleStruct->zmove.viewing = FALSE;
        MoveSelectionDestroyCursorAt(battler);
        sUsingCompactMoveList[battler] = TRUE;
        MoveSelectionDisplayMoveNames(battler);
        MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
        if (B_SHOW_EFFECTIVENESS)
            MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
        MoveSelectionDisplayPPNumber(battler);
        MoveSelectionDisplayMoveType(battler);
    }
}

static u32 UNUSED HandleMoveInputUnused(enum BattlerId battler)
{
    u32 var = 0;

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        var = 1;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        gBattle_BG0_X = 0;
        gBattle_BG0_Y = DISPLAY_HEIGHT * 2;
        var = 0xFF;
    }
    if (JOY_NEW(DPAD_LEFT) && gMoveSelectionCursor[battler] & 1)
    {
        MoveSelectionDestroyCursorAt(gMoveSelectionCursor[battler]);
        gMoveSelectionCursor[battler] ^= 1;
        PlaySE(SE_SELECT);
        MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
    }
    if (JOY_NEW(DPAD_RIGHT) && !(gMoveSelectionCursor[battler] & 1)
        && (gMoveSelectionCursor[battler] ^ 1) < gNumberOfMovesToChoose)
    {
        MoveSelectionDestroyCursorAt(gMoveSelectionCursor[battler]);
        gMoveSelectionCursor[battler] ^= 1;
        PlaySE(SE_SELECT);
        MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
    }
    if (JOY_NEW(DPAD_UP) && gMoveSelectionCursor[battler] & 2)
    {
        MoveSelectionDestroyCursorAt(gMoveSelectionCursor[battler]);
        gMoveSelectionCursor[battler] ^= 2;
        PlaySE(SE_SELECT);
        MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
    }
    if (JOY_NEW(DPAD_DOWN) && !(gMoveSelectionCursor[battler] & 2)
        && (gMoveSelectionCursor[battler] ^ 2) < gNumberOfMovesToChoose)
    {
        MoveSelectionDestroyCursorAt(gMoveSelectionCursor[battler]);
        gMoveSelectionCursor[battler] ^= 2;
        PlaySE(SE_SELECT);
        MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
    }

    return var;
}

void HandleMoveSwitching(enum BattlerId battler)
{
    u8 perMovePPBonuses[MAX_MON_MOVES];
    struct ChooseMoveStruct moveStruct;
    u8 totalPPBonuses;

    if (JOY_NEW(A_BUTTON | SELECT_BUTTON))
    {
        struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
        PlaySE(SE_SELECT);

        if (gMoveSelectionCursor[battler] != gMultiUsePlayerCursor)
        {
            struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
            s32 i;

            // swap moves and pp
            i = moveInfo->moves[gMoveSelectionCursor[battler]];
            moveInfo->moves[gMoveSelectionCursor[battler]] = moveInfo->moves[gMultiUsePlayerCursor];
            moveInfo->moves[gMultiUsePlayerCursor] = i;

            i = moveInfo->currentPP[gMoveSelectionCursor[battler]];
            moveInfo->currentPP[gMoveSelectionCursor[battler]] = moveInfo->currentPP[gMultiUsePlayerCursor];
            moveInfo->currentPP[gMultiUsePlayerCursor] = i;

            i = moveInfo->maxPP[gMoveSelectionCursor[battler]];
            moveInfo->maxPP[gMoveSelectionCursor[battler]] = moveInfo->maxPP[gMultiUsePlayerCursor];
            moveInfo->maxPP[gMultiUsePlayerCursor] = i;

            if (gBattleMons[battler].volatiles.mimickedMoves & (1u << gMoveSelectionCursor[battler]))
            {
                gBattleMons[battler].volatiles.mimickedMoves &= ~(1u << gMoveSelectionCursor[battler]);
                gBattleMons[battler].volatiles.mimickedMoves |= 1u << gMultiUsePlayerCursor;
            }

            MoveSelectionDisplayMoveNames(battler);

            for (i = 0; i < MAX_MON_MOVES; i++)
                perMovePPBonuses[i] = (gBattleMons[battler].ppBonuses & (3 << (i * 2))) >> (i * 2);

            totalPPBonuses = perMovePPBonuses[gMoveSelectionCursor[battler]];
            perMovePPBonuses[gMoveSelectionCursor[battler]] = perMovePPBonuses[gMultiUsePlayerCursor];
            perMovePPBonuses[gMultiUsePlayerCursor] = totalPPBonuses;

            totalPPBonuses = 0;
            for (i = 0; i < MAX_MON_MOVES; i++)
                totalPPBonuses |= perMovePPBonuses[i] << (i * 2);

            gBattleMons[battler].ppBonuses = totalPPBonuses;

            for (i = 0; i < MAX_MON_MOVES; i++)
            {
                gBattleMons[battler].moves[i] = moveInfo->moves[i];
                gBattleMons[battler].pp[i] = moveInfo->currentPP[i];
            }

            if (!(gBattleMons[battler].volatiles.transformed))
            {
                for (i = 0; i < MAX_MON_MOVES; i++)
                {
                    moveStruct.moves[i] = GetMonData(GetBattlerMon(battler), MON_DATA_MOVE1 + i);
                    moveStruct.currentPP[i] = GetMonData(GetBattlerMon(battler), MON_DATA_PP1 + i);
                }

                totalPPBonuses = GetMonData(GetBattlerMon(battler), MON_DATA_PP_BONUSES);
                for (i = 0; i < MAX_MON_MOVES; i++)
                    perMovePPBonuses[i] = (totalPPBonuses & (3 << (i * 2))) >> (i * 2);

                i = moveStruct.moves[gMoveSelectionCursor[battler]];
                moveStruct.moves[gMoveSelectionCursor[battler]] = moveStruct.moves[gMultiUsePlayerCursor];
                moveStruct.moves[gMultiUsePlayerCursor] = i;

                i = moveStruct.currentPP[gMoveSelectionCursor[battler]];
                moveStruct.currentPP[gMoveSelectionCursor[battler]] = moveStruct.currentPP[gMultiUsePlayerCursor];
                moveStruct.currentPP[gMultiUsePlayerCursor] = i;

                totalPPBonuses = perMovePPBonuses[gMoveSelectionCursor[battler]];
                perMovePPBonuses[gMoveSelectionCursor[battler]] = perMovePPBonuses[gMultiUsePlayerCursor];
                perMovePPBonuses[gMultiUsePlayerCursor] = totalPPBonuses;

                totalPPBonuses = 0;
                for (i = 0; i < MAX_MON_MOVES; i++)
                    totalPPBonuses |= perMovePPBonuses[i] << (i * 2);

                for (i = 0; i < MAX_MON_MOVES; i++)
                {
                    SetMonData(GetBattlerMon(battler), MON_DATA_MOVE1 + i, &moveStruct.moves[i]);
                    SetMonData(GetBattlerMon(battler), MON_DATA_PP1 + i, &moveStruct.currentPP[i]);
                }

                SetMonData(GetBattlerMon(battler), MON_DATA_PP_BONUSES, &totalPPBonuses);
            }
        }

        if (IS_FRLG && gBattleTypeFlags & BATTLE_TYPE_FIRST_BATTLE)
            gBattlerControllerFuncs[battler] = OakOldManHandleInputChooseMove;
        else
            gBattlerControllerFuncs[battler] = HandleInputChooseMove;
        gMoveSelectionCursor[battler] = gMultiUsePlayerCursor;
        MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
        if (B_SHOW_EFFECTIVENESS)
            MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
        else
            MoveSelectionDisplayPPString(battler);
        MoveSelectionDisplayPPNumber(battler);
        MoveSelectionDisplayMoveType(battler);
        AssignUsableZMoves(battler, moveInfo->moves);
    }
    else if (JOY_NEW(B_BUTTON | SELECT_BUTTON))
    {
        PlaySE(SE_SELECT);
        MoveSelectionDestroyCursorAt(gMultiUsePlayerCursor);
        MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);

        if (gBattleTypeFlags & BATTLE_TYPE_FIRST_BATTLE)
            gBattlerControllerFuncs[battler] = OakOldManHandleInputChooseMove;
        else
            gBattlerControllerFuncs[battler] = HandleInputChooseMove;

        if (B_SHOW_EFFECTIVENESS)
            MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
        else
            MoveSelectionDisplayPPString(battler);
        MoveSelectionDisplayPPNumber(battler);
        MoveSelectionDisplayMoveType(battler);
    }
    else if (JOY_NEW(DPAD_LEFT))
    {
        if (gMultiUsePlayerCursor & 1)
        {
            if (gMultiUsePlayerCursor == gMoveSelectionCursor[battler])
                MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 29);
            else
                MoveSelectionDestroyCursorAt(gMultiUsePlayerCursor);

            gMultiUsePlayerCursor ^= 1;
            PlaySE(SE_SELECT);

            if (gMultiUsePlayerCursor == gMoveSelectionCursor[battler])
                MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 0);
            else
                MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 27);
        }
    }
    else if (JOY_NEW(DPAD_RIGHT))
    {
        if (!(gMultiUsePlayerCursor & 1) && (gMultiUsePlayerCursor ^ 1) < gNumberOfMovesToChoose)
        {
            if (gMultiUsePlayerCursor == gMoveSelectionCursor[battler])
                MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 29);
            else
                MoveSelectionDestroyCursorAt(gMultiUsePlayerCursor);

            gMultiUsePlayerCursor ^= 1;
            PlaySE(SE_SELECT);

            if (gMultiUsePlayerCursor == gMoveSelectionCursor[battler])
                MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 0);
            else
                MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 27);
        }
    }
    else if (JOY_NEW(DPAD_UP))
    {
        if (gMultiUsePlayerCursor & 2)
        {
            if (gMultiUsePlayerCursor == gMoveSelectionCursor[battler])
                MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 29);
            else
                MoveSelectionDestroyCursorAt(gMultiUsePlayerCursor);

            gMultiUsePlayerCursor ^= 2;
            PlaySE(SE_SELECT);

            if (gMultiUsePlayerCursor == gMoveSelectionCursor[battler])
                MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 0);
            else
                MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 27);
        }
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (!(gMultiUsePlayerCursor & 2) && (gMultiUsePlayerCursor ^ 2) < gNumberOfMovesToChoose)
        {
            if (gMultiUsePlayerCursor == gMoveSelectionCursor[battler])
                MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 29);
            else
                MoveSelectionDestroyCursorAt(gMultiUsePlayerCursor);

            gMultiUsePlayerCursor ^= 2;
            PlaySE(SE_SELECT);

            if (gMultiUsePlayerCursor == gMoveSelectionCursor[battler])
                MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 0);
            else
                MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 27);
        }
    }
}

static void SetLinkBattleEndCallbacks(enum BattlerId battler)
{
    if (gWirelessCommType == 0)
    {
        if (!gReceivedRemoteLinkPlayers)
        {
            m4aSongNumStop(SE_LOW_HEALTH);
            gMain.inBattle = FALSE;
            gMain.callback1 = gPreBattleCallback1;
            SetMainCallback2(CB2_InitEndLinkBattle);
            if (gBattleOutcome == B_OUTCOME_WON)
                TryPutLinkBattleTvShowOnAir();
            FreeAllWindowBuffers();
        }
    }
    else
    {
        if (IsLinkTaskFinished())
        {
            m4aSongNumStop(SE_LOW_HEALTH);
            gMain.inBattle = FALSE;
            gMain.callback1 = gPreBattleCallback1;
            SetMainCallback2(CB2_InitEndLinkBattle);
            if (gBattleOutcome == B_OUTCOME_WON)
                TryPutLinkBattleTvShowOnAir();
            FreeAllWindowBuffers();
        }
    }
}

// Despite handling link battles separately, this is only ever used by link battles
void SetBattleEndCallbacks(enum BattlerId battler)
{
    if (!gPaletteFade.active)
    {
        if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        {
            if (IsLinkTaskFinished())
            {
                if (gWirelessCommType == 0)
                    SetCloseLinkCallback();
                else
                    SetLinkStandbyCallback();

                gBattlerControllerFuncs[battler] = SetLinkBattleEndCallbacks;
            }
        }
        else
        {
            m4aSongNumStop(SE_LOW_HEALTH);
            gMain.inBattle = FALSE;
            gMain.callback1 = gPreBattleCallback1;
            SetMainCallback2(gMain.savedCallback);
        }
    }
}

static void Intro_WaitForShinyAnimAndHealthbox(enum BattlerId battler)
{
    bool8 healthboxAnimDone = FALSE;

    // Check if healthbox has finished sliding in
    if (TwoPlayerIntroMons(battler) && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
    {
        if (gSprites[gHealthboxSpriteIds[battler]].callback == SpriteCallbackDummy
         && gSprites[gHealthboxSpriteIds[GetPartnerBattler(battler)]].callback == SpriteCallbackDummy)
            healthboxAnimDone = TRUE;
    }
    else
    {
        if (gSprites[gHealthboxSpriteIds[battler]].callback == SpriteCallbackDummy)
            healthboxAnimDone = TRUE;
    }

    // If healthbox and shiny anim are done
    if (healthboxAnimDone && gBattleSpritesDataPtr->healthBoxesData[battler].finishedShinyMonAnim
        && gBattleSpritesDataPtr->healthBoxesData[GetPartnerBattler(battler)].finishedShinyMonAnim)
    {
        // Reset shiny anim (even if it didn't occur)
        gBattleSpritesDataPtr->healthBoxesData[battler].triedShinyMonAnim = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[battler].finishedShinyMonAnim = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[GetPartnerBattler(battler)].triedShinyMonAnim = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[GetPartnerBattler(battler)].finishedShinyMonAnim = FALSE;
        FreeShinyStars();

        HandleLowHpMusicChange(GetBattlerMon(battler), battler);

        if (TwoPlayerIntroMons(battler))
            HandleLowHpMusicChange(GetBattlerMon(GetPartnerBattler(battler)), GetPartnerBattler(battler));

        gBattleSpritesDataPtr->healthBoxesData[battler].introEndDelay = 3;
        gBattlerControllerFuncs[battler] = BtlController_Intro_DelayAndEnd;
    }
}

static void Intro_TryShinyAnimShowHealthbox(enum BattlerId battler)
{
    bool32 bgmRestored = FALSE;
    bool32 battlerAnimsDone = FALSE;

    // Start shiny animation if applicable for 1st Pokémon
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].triedShinyMonAnim
     && !gBattleSpritesDataPtr->healthBoxesData[battler].ballAnimActive)
        TryShinyAnimation(battler, GetBattlerMon(battler));

    // Start shiny animation if applicable for 2nd Pokémon
    if (!gBattleSpritesDataPtr->healthBoxesData[GetPartnerBattler(battler)].triedShinyMonAnim
     && !gBattleSpritesDataPtr->healthBoxesData[GetPartnerBattler(battler)].ballAnimActive)
        TryShinyAnimation(GetPartnerBattler(battler), GetBattlerMon(GetPartnerBattler(battler)));

    // Show healthbox after ball anim
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].ballAnimActive
     && !gBattleSpritesDataPtr->healthBoxesData[GetPartnerBattler(battler)].ballAnimActive)
    {
        if (!gBattleSpritesDataPtr->healthBoxesData[battler].healthboxSlideInStarted)
        {
            if (TwoPlayerIntroMons(battler) && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
            {
                UpdateHealthboxAttribute(gHealthboxSpriteIds[GetPartnerBattler(battler)], GetBattlerMon(GetPartnerBattler(battler)), HEALTHBOX_ALL);
                StartHealthboxSlideIn(GetPartnerBattler(battler));
                SetHealthboxSpriteVisible(gHealthboxSpriteIds[GetPartnerBattler(battler)]);
            }
            UpdateHealthboxAttribute(gHealthboxSpriteIds[battler], GetBattlerMon(battler), HEALTHBOX_ALL);
            StartHealthboxSlideIn(battler);
            SetHealthboxSpriteVisible(gHealthboxSpriteIds[battler]);
        }
        gBattleSpritesDataPtr->healthBoxesData[battler].healthboxSlideInStarted = TRUE;
    }

    // Restore bgm after cry has played and healthbox anim is started
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].waitForCry
        && gBattleSpritesDataPtr->healthBoxesData[battler].healthboxSlideInStarted
        && !gBattleSpritesDataPtr->healthBoxesData[GetPartnerBattler(battler)].waitForCry
        && !IsCryPlayingOrClearCrySongs())
    {
        if (!gBattleSpritesDataPtr->healthBoxesData[battler].bgmRestored)
        {
            if (gBattleTypeFlags & BATTLE_TYPE_MULTI && gBattleTypeFlags & BATTLE_TYPE_LINK)
                m4aMPlayContinue(&gMPlayInfo_BGM);
            else
                m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 0x100);
        }
        gBattleSpritesDataPtr->healthBoxesData[battler].bgmRestored = TRUE;
        bgmRestored = TRUE;
    }

    // Wait for battler anims
    if (TwoPlayerIntroMons(battler) && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
    {
        if (gSprites[gBattleControllerData[battler]].callback == SpriteCallbackDummy
            && gSprites[gBattlerSpriteIds[battler]].callback == SpriteCallbackDummy
            && gSprites[gBattleControllerData[GetPartnerBattler(battler)]].callback == SpriteCallbackDummy
            && gSprites[gBattlerSpriteIds[GetPartnerBattler(battler)]].callback == SpriteCallbackDummy)
        {
            battlerAnimsDone = TRUE;
        }
    }
    else
    {
        if (gSprites[gBattleControllerData[battler]].callback == SpriteCallbackDummy
            && gSprites[gBattlerSpriteIds[battler]].callback == SpriteCallbackDummy)
        {
            battlerAnimsDone = TRUE;
        }
    }

    // Clean up
    if (bgmRestored && battlerAnimsDone)
    {
        if (TwoPlayerIntroMons(battler) && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
            DestroySprite(&gSprites[gBattleControllerData[GetPartnerBattler(battler)]]);
        DestroySprite(&gSprites[gBattleControllerData[battler]]);

        gBattleSpritesDataPtr->animationData->introAnimActive = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[battler].bgmRestored = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[battler].healthboxSlideInStarted = FALSE;

        gBattlerControllerFuncs[battler] = Intro_WaitForShinyAnimAndHealthbox;
    }
}

void Task_PlayerController_RestoreBgmAfterCry(u8 taskId)
{
    if (!IsCryPlayingOrClearCrySongs())
    {
        m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 0x100);
        DestroyTask(taskId);
    }
}

#define tExpTask_monId          data[0]
#define tExpTask_battler        data[2]
#define tExpTask_gainedExp_1    data[3]
#define tExpTask_gainedExp_2    data[4] // Stored as two half-words containing a word.
#define tExpTask_frames         data[10]

static void DynamaxModifyHPLevelUp(struct Pokemon *mon, enum BattlerId battler, u32 oldMaxHP)
{
    ApplyDynamaxHPMultiplier(mon);
    gBattleScripting.levelUpHP = GetMonData(mon, MON_DATA_MAX_HP) - oldMaxHP; // overwrite levelUpHP since it overflows
    gBattleMons[battler].hp += gBattleScripting.levelUpHP;
    SetMonData(mon, MON_DATA_HP, &gBattleMons[battler].hp);
}

static s32 GetTaskExpValue(u8 taskId)
{
    return (u16)(gTasks[taskId].tExpTask_gainedExp_1) | (gTasks[taskId].tExpTask_gainedExp_2 << 16);
}

static void Task_GiveExpToMon(u8 taskId)
{
    u32 monId = (u8)(gTasks[taskId].tExpTask_monId);
    enum BattlerId battler = gTasks[taskId].tExpTask_battler;
    s32 gainedExp = GetTaskExpValue(taskId);

    if (GetBattlerCoordsIndex(battler) == BATTLE_COORDS_DOUBLES || monId != gBattlerPartyIndexes[battler]) // Give exp without moving the expbar.
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][monId];
        enum Species species = GetMonData(mon, MON_DATA_SPECIES);
        u8 level = GetMonData(mon, MON_DATA_LEVEL);
        u32 currExp = GetMonData(mon, MON_DATA_EXP);
        u32 nextLvlExp = gExperienceTables[gSpeciesInfo[species].growthRate][level + 1];
        u32 expAfterGain = currExp + gainedExp;
        u32 oldMaxHP = GetMonData(mon, MON_DATA_MAX_HP);

        if (expAfterGain >= nextLvlExp)
        {
            SetMonData(mon, MON_DATA_EXP, (B_LEVEL_UP_NOTIFICATION >= GEN_9) ? &expAfterGain : &nextLvlExp);

            CalculateMonStats(mon);

            // Reapply Dynamax HP multiplier after stats are recalculated.
            if (GetActiveGimmick(battler) == GIMMICK_DYNAMAX && monId == gBattlerPartyIndexes[battler])
                DynamaxModifyHPLevelUp(mon, battler, oldMaxHP);

            gainedExp -= nextLvlExp - currExp;
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, RET_VALUE_LEVELED_UP, (B_LEVEL_UP_NOTIFICATION >= GEN_9) ? 0 : gainedExp);

            if (IsDoubleBattle() == TRUE
             && (monId == gBattlerPartyIndexes[battler] || monId == gBattlerPartyIndexes[GetPartnerBattler(battler)]))
                gTasks[taskId].func = Task_LaunchLvlUpAnim;
            else
                gTasks[taskId].func = Task_SetControllerToWaitForString;
        }
        else
        {
            currExp += gainedExp;
            SetMonData(mon, MON_DATA_EXP, &currExp);
            gBattlerControllerFuncs[battler] = Controller_WaitForString;
            DestroyTask(taskId);
        }
    }
    else
    {
        gTasks[taskId].func = Task_PrepareToGiveExpWithExpBar;
    }
}

static void Task_PrepareToGiveExpWithExpBar(u8 taskId)
{
    u8 monIndex = gTasks[taskId].tExpTask_monId;
    s32 gainedExp = GetTaskExpValue(taskId);
    enum BattlerId battler = gTasks[taskId].tExpTask_battler;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][monIndex];
    u8 level = GetMonData(mon, MON_DATA_LEVEL);
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    u32 exp = GetMonData(mon, MON_DATA_EXP);
    u32 currLvlExp = gExperienceTables[gSpeciesInfo[species].growthRate][level];
    u32 expToNextLvl;

    exp -= currLvlExp;
    expToNextLvl = gExperienceTables[gSpeciesInfo[species].growthRate][level + 1] - currLvlExp;
    SetBattleBarStruct(battler, gHealthboxSpriteIds[battler], expToNextLvl, exp, -gainedExp);
    TestRunner_Battle_RecordExp(battler, exp, -gainedExp);
    PlaySE(SE_EXP);
    gTasks[taskId].func = Task_GiveExpWithExpBar;
}

static void Task_GiveExpWithExpBar(u8 taskId)
{
    u32 level, expAfterGain;
    enum Species species;
    u32 oldMaxHP;
    s32 currExp, expOnNextLvl, newExpPoints;

    if (gTasks[taskId].tExpTask_frames < 13)
    {
        gTasks[taskId].tExpTask_frames++;
    }
    else
    {
        u8 monId = gTasks[taskId].tExpTask_monId;
        s32 gainedExp = GetTaskExpValue(taskId);
        enum BattlerId battler = gTasks[taskId].tExpTask_battler;
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][monId];

        newExpPoints = MoveBattleBar(battler, gHealthboxSpriteIds[battler], EXP_BAR, 0);
        SetHealthboxSpriteVisible(gHealthboxSpriteIds[battler]);
        if (newExpPoints == -1) // The bar has been filled with given exp points.
        {
            m4aSongNumStop(SE_EXP);
            level = GetMonData(mon, MON_DATA_LEVEL);
            currExp = GetMonData(mon, MON_DATA_EXP);
            species = GetMonData(mon, MON_DATA_SPECIES);
            oldMaxHP = GetMonData(mon, MON_DATA_MAX_HP);
            expOnNextLvl = gExperienceTables[gSpeciesInfo[species].growthRate][level + 1];

            expAfterGain = currExp + gainedExp;
            if (expAfterGain >= expOnNextLvl)
            {
                if (B_LEVEL_UP_NOTIFICATION >= GEN_9)
                    SetMonData(mon, MON_DATA_EXP, &expAfterGain);
                else
                    SetMonData(mon, MON_DATA_EXP, &expOnNextLvl);

                CalculateMonStats(mon);

                // Reapply Dynamax HP multiplier after stats are recalculated.
                if (GetActiveGimmick(battler) == GIMMICK_DYNAMAX && monId == gBattlerPartyIndexes[battler])
                    DynamaxModifyHPLevelUp(mon, battler, oldMaxHP);

                gainedExp -= expOnNextLvl - currExp;
                BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, RET_VALUE_LEVELED_UP, (B_LEVEL_UP_NOTIFICATION >= GEN_9) ? 0 : gainedExp);
                gTasks[taskId].func = Task_LaunchLvlUpAnim;
            }
            else
            {
                currExp += gainedExp;
                SetMonData(mon, MON_DATA_EXP, &currExp);
                gBattlerControllerFuncs[battler] = Controller_WaitForString;
                DestroyTask(taskId);
            }
        }
    }
}

static void Task_LaunchLvlUpAnim(u8 taskId)
{
    enum BattlerId battler = gTasks[taskId].tExpTask_battler;
    u8 monIndex = gTasks[taskId].tExpTask_monId;

    if (IsDoubleBattle() == TRUE && monIndex == gBattlerPartyIndexes[GetPartnerBattler(battler)])
        battler ^= BIT_FLANK;

    InitAndLaunchSpecialAnimation(battler, battler, battler, B_ANIM_LVL_UP);
    gTasks[taskId].func = Task_UpdateLvlInHealthbox;
}

static void Task_UpdateLvlInHealthbox(u8 taskId)
{
    enum BattlerId battler = gTasks[taskId].tExpTask_battler;

    if (!gBattleSpritesDataPtr->healthBoxesData[battler].specialAnimActive)
    {
        u8 monIndex = gTasks[taskId].tExpTask_monId;

        if (IsDoubleBattle() == TRUE && monIndex == gBattlerPartyIndexes[GetPartnerBattler(battler)])
            UpdateHealthboxAttribute(gHealthboxSpriteIds[GetPartnerBattler(battler)], &gParties[B_TRAINER_PLAYER][monIndex], HEALTHBOX_ALL);
        else
            UpdateHealthboxAttribute(gHealthboxSpriteIds[battler], &gParties[B_TRAINER_PLAYER][monIndex], HEALTHBOX_ALL);

        gTasks[taskId].func = Task_SetControllerToWaitForString;
    }
}

static void Task_SetControllerToWaitForString(u8 taskId)
{
    enum BattlerId battler = gTasks[taskId].tExpTask_battler;
    gBattlerControllerFuncs[battler] = Controller_WaitForString;
    DestroyTask(taskId);
}

static void OpenPartyMenuToChooseMon(enum BattlerId battler)
{
    if (!gPaletteFade.active)
    {
        u8 caseId;

        gBattlerControllerFuncs[battler] = WaitForMonSelection;
        caseId = gTasks[gBattleControllerData[battler]].data[0];
        DestroyTask(gBattleControllerData[battler]);
        CloseMainBattleScreen();
        OpenPartyMenuInBattle(caseId);
    }
}

static void WaitForMonSelection(enum BattlerId battler)
{
    if (gMain.callback2 == BattleMainCB2 && !gPaletteFade.active)
    {
        if (gPartyMenuUseExitCallback == TRUE)
            BtlController_EmitChosenMonReturnValue(battler, B_COMM_TO_ENGINE, gSelectedMonPartyId, gBattlePartyCurrentOrder);
        else
            BtlController_EmitChosenMonReturnValue(battler, B_COMM_TO_ENGINE, PARTY_SIZE, NULL);

        if (gBattleResources->bufferA[battler][1] == PARTY_ACTION_SEND_OUT)
            PrintLinkStandbyMsg();

        BtlController_Complete(battler);
    }
}

static void OpenBagAndChooseItem(enum BattlerId battler)
{
    if (!gPaletteFade.active)
    {
        gBattlerControllerFuncs[battler] = CompleteWhenChoseItem;
        ReshowBattleScreenDummy();
        CloseMainBattleScreen();
        CB2_BagMenuFromBattle();
    }
}

static void CompleteWhenChoseItem(enum BattlerId battler)
{
    if (gMain.callback2 == BattleMainCB2 && !gPaletteFade.active)
    {
        BtlController_EmitOneReturnValue(battler, B_COMM_TO_ENGINE, gSpecialVar_ItemId);
        BtlController_Complete(battler);
    }
}

static void PlayerHandleYesNoInput(enum BattlerId battler)
{
    if (JOY_NEW(DPAD_UP) && gMultiUsePlayerCursor != 0)
    {
        PlaySE(SE_SELECT);
        BattleDestroyYesNoCursorAt(gMultiUsePlayerCursor);
        gMultiUsePlayerCursor = 0;
        BattleCreateYesNoCursorAt(0);
    }
    if (JOY_NEW(DPAD_DOWN) && gMultiUsePlayerCursor == 0)
    {
        PlaySE(SE_SELECT);
        BattleDestroyYesNoCursorAt(gMultiUsePlayerCursor);
        gMultiUsePlayerCursor = 1;
        BattleCreateYesNoCursorAt(1);
    }
    if (JOY_NEW(A_BUTTON))
    {
        HandleBattleWindow(YESNOBOX_X_Y, WINDOW_CLEAR);
        PlaySE(SE_SELECT);

        if (gMultiUsePlayerCursor != 0)
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_UNK_14, 0);
        else
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_NOTHING_FAINTED, 0);

        BtlController_Complete(battler);
    }
    if (JOY_NEW(B_BUTTON))
    {
        HandleBattleWindow(YESNOBOX_X_Y, WINDOW_CLEAR);
        PlaySE(SE_SELECT);
        BtlController_Complete(battler);
    }
}

static void MoveSelectionDisplayMoveNames(enum BattlerId battler)
{
    s32 i;
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
    static const u8 sPpStart[] = _("{CLEAR_TO 64}PP");
    static const u8 sSlash[] = _("/");
    if (!gBattleStruct->zmove.viewing)
    {
        DrawCompactMoveList(battler);
        return;
    }

    gNumberOfMovesToChoose = 0;

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        MoveSelectionDestroyCursorAt(i);
        if (IsGimmickSelected(battler, GIMMICK_DYNAMAX) || GetActiveGimmick(battler) == GIMMICK_DYNAMAX)
            StringCopy(gDisplayedStringBattle, GetMoveName(GetMaxMove(battler, moveInfo->moves[i])));
        else
            StringCopy(gDisplayedStringBattle, GetMoveName(moveInfo->moves[i]));

        if (!gBattleStruct->zmove.viewing)
        {
            u8 *txtPtr = StringAppend(gDisplayedStringBattle, sPpStart);
            ConvertIntToDecimalStringN(txtPtr, moveInfo->currentPP[i], STR_CONV_MODE_RIGHT_ALIGN, 2);
            txtPtr = StringAppend(gDisplayedStringBattle, sSlash);
            ConvertIntToDecimalStringN(txtPtr, moveInfo->maxPP[i], STR_CONV_MODE_RIGHT_ALIGN, 2);
        }

        BattlePutTextOnWindow(gDisplayedStringBattle, i + B_WIN_MOVE_NAME_1);
        if (moveInfo->moves[i] != MOVE_NONE)
            gNumberOfMovesToChoose++;
    }
}

static void MoveSelectionDisplayPPString(enum BattlerId battler)
{
    if (sUsingCompactMoveList[battler])
        return;

    StringCopy(gDisplayedStringBattle, gText_MoveInterfacePP);
    BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_PP);
}

static void MoveSelectionDisplayPPNumber(enum BattlerId battler)
{
    u8 *txtPtr;
    struct ChooseMoveStruct *moveInfo;

    if (sUsingCompactMoveList[battler] || gBattleResources->bufferA[battler][2] == TRUE) // check if we didn't want to display pp number
        return;

    SetPPNumbersPaletteInMoveSelection(battler);
    moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
    txtPtr = ConvertIntToDecimalStringN(gDisplayedStringBattle, moveInfo->currentPP[gMoveSelectionCursor[battler]], STR_CONV_MODE_RIGHT_ALIGN, 2);
    *(txtPtr)++ = CHAR_SLASH;
    ConvertIntToDecimalStringN(txtPtr, moveInfo->maxPP[gMoveSelectionCursor[battler]], STR_CONV_MODE_RIGHT_ALIGN, 2);

    BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_PP_REMAINING);
}

static void MoveSelectionDisplayMoveType(enum BattlerId battler)
{
    u8 *txtPtr, *end;

    if (sUsingCompactMoveList[battler])
        return;
    enum Species speciesId = gBattleMons[battler].species;
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
    txtPtr = StringCopy(gDisplayedStringBattle, gText_MoveInterfaceType);
    enum Move move = moveInfo->moves[gMoveSelectionCursor[battler]];
    enum Type type = GetMoveType(move);
    enum BattleMoveEffects effect = GetMoveEffect(move);

    if (effect == EFFECT_TERA_BLAST)
    {
        if (IsGimmickSelected(battler, GIMMICK_TERA) || GetActiveGimmick(battler) == GIMMICK_TERA)
            type = GetBattlerTeraType(battler);
    }
    else if (effect == EFFECT_IVY_CUDGEL)
    {
        if (speciesId == SPECIES_OGERPON_WELLSPRING || speciesId == SPECIES_OGERPON_WELLSPRING_TERA
         || speciesId == SPECIES_OGERPON_HEARTHFLAME || speciesId == SPECIES_OGERPON_HEARTHFLAME_TERA
         || speciesId == SPECIES_OGERPON_CORNERSTONE || speciesId == SPECIES_OGERPON_CORNERSTONE_TERA)
            type = GetSpeciesType(speciesId, 1);
    }
    else if (GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS
             && (GetActiveGimmick(battler) == GIMMICK_DYNAMAX || IsGimmickSelected(battler, GIMMICK_DYNAMAX)))
    {
        type = TYPE_NORMAL; // Max Guard is always a Normal-type move
    }
    else if (effect == EFFECT_TERA_STARSTORM)
    {
        if (speciesId == SPECIES_TERAPAGOS_STELLAR
        || (IsGimmickSelected(battler, GIMMICK_TERA) && speciesId == SPECIES_TERAPAGOS_TERASTAL))
            type = TYPE_STELLAR;
    }
    else if (P_SHOW_DYNAMIC_TYPES) // Non-vanilla changes to battle UI showing dynamic types
    {
        struct Pokemon *mon = GetBattlerMon(battler);
        type = CheckDynamicMoveType(mon, move, battler, MON_IN_BATTLE);
    }
    end = StringCopy(txtPtr, gTypesInfo[type].name);

    PrependFontIdToFit(txtPtr, end, FONT_NORMAL, WindowWidthPx(B_WIN_MOVE_TYPE) - 25);
    BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MOVE_TYPE);
}

static void TryMoveSelectionDisplayMoveDescription(enum BattlerId battler)
{
    if (!B_SHOW_MOVE_DESCRIPTION)
        return;

    MoveSelectionDisplayMoveDescription(battler);
}

static void MoveSelectionDisplayMoveDescription(enum BattlerId battler)
{
    if (sUsingCompactMoveList[battler])
    {
        DrawCompactMoveInfo(battler);
        return;
    }

    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct*)(&gBattleResources->bufferA[battler][4]);
    enum Move move = moveInfo->moves[gMoveSelectionCursor[battler]];
    u16 pwr = GetMovePower(move);
    u16 acc = GetMoveAccuracy(move);
    enum Type type;

    if (GetActiveGimmick(battler) == GIMMICK_DYNAMAX || IsGimmickSelected(battler, GIMMICK_DYNAMAX))
    {
        pwr = GetMaxMovePower(move, move);
        move = GetMaxMove(battler, move);
        acc = 0;
    }

    type = GetMoveType(move);

    u8 pwr_num[3], acc_num[3];
    u8 pwr_desc[6] = _("PWR: ");
    u8 acc_desc[6] = _("ACC: ");
    u8 type_desc[7] = _("TYPE: ");
    static const u8 sSpace[] = _(" ");
    if (pwr < 2)
        StringCopy(pwr_num, gText_BattleSwitchWhich5);
    else
        ConvertIntToDecimalStringN(pwr_num, pwr, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (acc < 2)
        StringCopy(acc_num, gText_BattleSwitchWhich5);
    else
        ConvertIntToDecimalStringN(acc_num, acc, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringCopy(gDisplayedStringBattle, GetMoveDescription(move));
    StringAppend(gDisplayedStringBattle, gText_NewLine);
    StringAppend(gDisplayedStringBattle, pwr_desc);
    StringAppend(gDisplayedStringBattle, pwr_num);
    StringAppend(gDisplayedStringBattle, sSpace);
    StringAppend(gDisplayedStringBattle, acc_desc);
    StringAppend(gDisplayedStringBattle, acc_num);
    StringAppend(gDisplayedStringBattle, gText_NewLine);
    StringAppend(gDisplayedStringBattle, type_desc);
    StringAppend(gDisplayedStringBattle, gTypesInfo[type].name);
    BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MOVE_DESCRIPTION);
}

static const u8 *BuildCompactDescriptionLine(const u8 *src, u8 *dst, u16 maxWidth)
{
    u8 *lineEnd = dst;

    while (*src == CHAR_SPACE || *src == CHAR_NEWLINE)
        src++;

    while (*src != EOS)
    {
        const u8 *wordStart = src;
        u8 *previousLineEnd = lineEnd;

        if (lineEnd != dst)
            *lineEnd++ = CHAR_SPACE;

        while (*src != CHAR_SPACE && *src != CHAR_NEWLINE && *src != EOS)
            *lineEnd++ = *src++;
        *lineEnd = EOS;

        if (GetStringWidth(FONT_COMPACT, dst, 0) > maxWidth && previousLineEnd != dst)
        {
            *previousLineEnd = EOS;
            return wordStart;
        }

        while (*src == CHAR_SPACE || *src == CHAR_NEWLINE)
            src++;
    }

    return src;
}

static void DrawCompactEffectivenessIndicator(u32 effectiveness, u16 x)
{
    u8 color;
    u16 y = 16;

    switch (effectiveness)
    {
    case EFFECTIVENESS_SUPER_EFFECTIVE:
        color = COMPACT_ARROW_GREEN;
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x + 3, y,     2, 1);
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x + 2, y + 1, 4, 1);
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x + 1, y + 2, 6, 1);
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x,     y + 3, 8, 1);
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x + 3, y + 4, 2, 4);
        break;
    case EFFECTIVENESS_NOT_VERY_EFFECTIVE:
        color = COMPACT_ARROW_RED;
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x + 3, y,     2, 4);
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x,     y + 4, 8, 1);
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x + 1, y + 5, 6, 1);
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x + 2, y + 6, 4, 1);
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x + 3, y + 7, 2, 1);
        break;
    case EFFECTIVENESS_NO_EFFECT:
        color = COMPACT_ARROW_GRAY;
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x,     y,     2, 1);
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x + 6, y,     2, 1);
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x + 1, y + 1, 2, 1);
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x + 5, y + 1, 2, 1);
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x + 2, y + 2, 4, 2);
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x + 1, y + 4, 2, 1);
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x + 5, y + 4, 2, 1);
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x,     y + 5, 2, 1);
        FillWindowPixelRect(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(color), x + 6, y + 5, 2, 1);
        break;
    }
}

static void DrawCompactMoveInfo(enum BattlerId battler)
{
    static const u8 sPwr[] = _("P:");
    static const u8 sAcc[] = _(" A:");
    static const u8 sType[] = _(" T:");
    const u8 *description;
    u8 *dst;
    u16 arrowX;
    s32 line;
    u32 foeEffectiveness;
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
    enum Move move = moveInfo->moves[gMoveSelectionCursor[battler]];
    u16 pwr = GetMovePower(move);
    u16 acc = GetMoveAccuracy(move);
    enum Type type;

    if (GetActiveGimmick(battler) == GIMMICK_DYNAMAX || IsGimmickSelected(battler, GIMMICK_DYNAMAX))
    {
        pwr = GetMaxMovePower(move, move);
        move = GetMaxMove(battler, move);
        acc = 0;
    }

    type = GetMoveType(move);
    foeEffectiveness = EFFECTIVENESS_CANNOT_VIEW;
    if (!IsBattleMoveStatus(move))
        foeEffectiveness = CheckCompactTargetTypeEffectiveness(battler);

    FillWindowPixelBuffer(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(0xE));

    // Keep the description to two rows with a 2px gap between their glyphs.
    description = GetMoveDescription(move);
    for (line = 0; line < 2 && *description != EOS; line++)
    {
        description = BuildCompactDescriptionLine(description, gDisplayedStringBattle,
                                                   WindowWidthPx(B_WIN_MOVE_DESCRIPTION));
        AddTextPrinterParameterized4(B_WIN_MOVE_DESCRIPTION, FONT_COMPACT, 0, line * 8, 0, 0,
                                     sCompactMoveTextColors, TEXT_SKIP_DRAW, gDisplayedStringBattle);
    }

    dst = StringCopy(gDisplayedStringBattle, sPwr);
    dst = ConvertIntToDecimalStringN(dst, pwr, STR_CONV_MODE_LEFT_ALIGN, 3);
    dst = StringAppend(dst, sAcc);
    dst = ConvertIntToDecimalStringN(dst, acc, STR_CONV_MODE_LEFT_ALIGN, 3);
    *dst = EOS;
    dst = StringAppend(dst, sType);
    dst = StringAppend(dst, gTypesInfo[type].name);
    *dst = EOS;
    AddTextPrinterParameterized4(B_WIN_MOVE_DESCRIPTION, FONT_COMPACT, 0, 16, 0, 0,
                                 sCompactMoveTextColors, TEXT_SKIP_DRAW, gDisplayedStringBattle);

    arrowX = GetStringWidth(FONT_COMPACT, gDisplayedStringBattle, 0) + 2;
    DrawCompactEffectivenessIndicator(foeEffectiveness, arrowX);

    PutWindowTilemap(B_WIN_MOVE_DESCRIPTION);
    CopyWindowToVram(B_WIN_MOVE_DESCRIPTION, COPYWIN_FULL);
}

static bool32 IsAnyCompactMoveListActive(void)
{
    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        if (sUsingCompactMoveList[battler])
            return TRUE;
    }

    return FALSE;
}

void MoveSelectionCreateCursorAt(u8 cursorPosition, u8 baseTileNum)
{
    u16 src[2];

    if (IsAnyCompactMoveListActive())
        return;

    src[0] = baseTileNum + 1;
    src[1] = baseTileNum + 2;

    CopyToBgTilemapBufferRect_ChangePalette(0, src, 1, 51 + cursorPosition * 2, 1, 2, 0x11);
    CopyBgTilemapBufferToVram(0);
}

void MoveSelectionDestroyCursorAt(u8 cursorPosition)
{
    u16 src[2];

    if (IsAnyCompactMoveListActive())
        return;

    src[0] = 0x1016;
    src[1] = 0x1016;

    CopyToBgTilemapBufferRect_ChangePalette(0, src, 1, 51 + cursorPosition * 2, 1, 2, 0x11);
    CopyBgTilemapBufferToVram(0);
}

void ActionSelectionCreateCursorAt(u8 cursorPosition, u8 baseTileNum)
{
    u16 src[2];
    if (sUsingModernActionMenu)
        return;

    src[0] = 1;
    src[1] = 2;

    CopyToBgTilemapBufferRect_ChangePalette(0, src, 7 * (cursorPosition & 1) + 16, 35 + (cursorPosition & 2), 1, 2, 0x11);
    CopyBgTilemapBufferToVram(0);
}

void ActionSelectionDestroyCursorAt(u8 cursorPosition)
{
    u16 src[2];
    if (sUsingModernActionMenu)
        return;

    src[0] = 0x1016;
    src[1] = 0x1016;

    CopyToBgTilemapBufferRect_ChangePalette(0, src, 7 * (cursorPosition & 1) + 16, 35 + (cursorPosition & 2), 1, 2, 0x11);
    CopyBgTilemapBufferToVram(0);
}

void CB2_SetUpReshowBattleScreenAfterMenu(void)
{
    SetMainCallback2(ReshowBattleScreenAfterMenu);
}

void CB2_SetUpReshowBattleScreenAfterMenu2(void)
{
    SetMainCallback2(ReshowBattleScreenAfterMenu);
}

static void PrintLinkStandbyMsg(void)
{
    if (gBattleTypeFlags & BATTLE_TYPE_LINK)
    {
        gBattle_BG0_X = 0;
        gBattle_BG0_Y = 0;
        BattlePutTextOnWindow(gText_LinkStandby, B_WIN_MSG);
    }
}

static void PlayerHandleLoadMonSprite(enum BattlerId battler)
{
    BattleLoadMonSpriteGfx(GetBattlerMon(battler), battler);
    gSprites[gBattlerSpriteIds[battler]].oam.paletteNum = battler;
    gBattlerControllerFuncs[battler] = CompleteOnBattlerSpritePosX_0;
}

enum TrainerPicID LinkPlayerGetTrainerPicId(u32 multiplayerId)
{
    u8 gender = gLinkPlayers[multiplayerId].gender;
    enum GameVersion version = gLinkPlayers[multiplayerId].version & 0xFF;

    return GetPlayerTrainerPic(gender, version);
}

static enum TrainerPicID PlayerGetTrainerBackPicId(void)
{
    enum TrainerPicID trainerPicId;

    if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        trainerPicId = LinkPlayerGetTrainerPicId(GetMultiplayerId());
    else
        trainerPicId = GetPlayerTrainerPic(gSaveBlock2Ptr->playerGender, GAME_VERSION);

    return trainerPicId;
}

// In emerald it's possible to have a tag battle in the battle frontier facilities with AI
// which use the front sprite for both the player and the partner as opposed to any other battles (including the one with Steven)
// that use an animated back pic.
static void PlayerHandleDrawTrainerPic(enum BattlerId battler)
{
    bool32 isFrontPic;
    s16 xPos, yPos;
    enum TrainerPicID trainerPicId;

    if (TESTING)
    {
        trainerPicId = TRAINER_PIC_BRENDAN;
        if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER)
            xPos = 32;
        else
            xPos = 80;
        yPos = (8 - GetTrainerBackPicCoords(trainerPicId)->size) * 4 + 80;
    }
    else
    {
        trainerPicId = PlayerGetTrainerBackPicId();

        if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
        {
            if ((GetBattlerPosition(battler) & BIT_FLANK) != B_FLANK_LEFT) // Second mon, on the right.
                xPos = 90;
            else // First mon, on the left.
                xPos = 32;

            if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER && gPartnerTrainerId < TRAINER_PARTNER(PARTNER_NONE))
            {
                xPos = 90;
                yPos = 80;
            }
            else
            {
                yPos = (8 - GetTrainerBackPicCoords(trainerPicId)->size) * 4 + 80;
            }
        }
        else
        {
            xPos = 80;
            yPos = (8 - GetTrainerBackPicCoords(trainerPicId)->size) * 4 + 80;
        }
    }

    // Use front pic table for any tag battles unless your partner is Steven or a custom partner.
    if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER && gPartnerTrainerId < TRAINER_PARTNER(PARTNER_NONE))
    {
        trainerPicId = PlayerGenderToFrontTrainerPicId(gSaveBlock2Ptr->playerGender);
        isFrontPic = TRUE;
    }
    else // Use back pic in all the other usual circumstances.
    {
        isFrontPic = FALSE;
    }

    BtlController_HandleDrawTrainerPic(battler, trainerPicId, isFrontPic, xPos, yPos, -1);
}

static void PlayerHandleTrainerSlide(enum BattlerId battler)
{
    enum TrainerPicID trainerPicId = PlayerGetTrainerBackPicId();
    BtlController_HandleTrainerSlide(battler, trainerPicId);
}

static void PlayerHandleTrainerSlideBack(enum BattlerId battler)
{
    BtlController_HandleTrainerSlideBack(battler, 50, TRUE);
}

static void PlayerHandlePaletteFade(enum BattlerId battler)
{
    BeginNormalPaletteFade(PALETTES_ALL, 2, 0, 16, RGB_BLACK);
    BtlController_Complete(battler);
}

static void PlayerHandlePause(enum BattlerId battler)
{
    u8 timer = gBattleResources->bufferA[battler][1];

    while (timer != 0)
        timer--;

    BtlController_Complete(battler);
}

static void HandleChooseActionAfterDma3(enum BattlerId battler)
{
    if (!IsDma3ManagerBusyWithBgCopy())
    {
        gBattle_BG0_X = 0;
        gBattle_BG0_Y = DISPLAY_HEIGHT;
        if (gBattleStruct->aiDelayTimer != 0)
        {
            if (DEBUG_AI_DELAY_TIMER)
            {
                static const u8 sFramesText[] = _(" frames thinking\n");
                static const u8 sCyclesText[] = _(" cycles");
                ConvertIntToDecimalStringN(gDisplayedStringBattle, gBattleStruct->aiDelayFrames, STR_CONV_MODE_RIGHT_ALIGN, 3);
                u8* end = StringAppend(gDisplayedStringBattle, sFramesText);
                ConvertIntToDecimalStringN(end, gBattleStruct->aiDelayCycles, STR_CONV_MODE_RIGHT_ALIGN, 8);
                // Clear old result once read out
                gBattleStruct->aiDelayCycles = 0;
                StringAppend(gDisplayedStringBattle, sCyclesText);
                BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_ACTION_PROMPT);
            }
            gBattleStruct->aiDelayTimer = 0;
            gBattleStruct->aiDelayFrames = 0;
        }
        gBattlerControllerFuncs[battler] = HandleInputChooseAction;
    }
}

static void PlayerHandleChooseAction(enum BattlerId battler)
{
    s32 i;

    // The normal command screen replaces the compact move/picker screen.
    sUsingCompactMoveList[battler] = FALSE;
    sUsingModernActionMenu = TRUE;
    sModernActionMenuPromptNeedsRefresh = TRUE;
    gBattlerControllerFuncs[battler] = HandleChooseActionAfterDma3;
    BattleTv_ClearExplosionFaintCause();

    for (i = 0; i < 4; i++)
        ActionSelectionDestroyCursorAt(i);

    TryRestoreLastUsedBall();
    ActionSelectionCreateCursorAt(gActionSelectionCursor[battler], 0);
    DrawModernActionMenu(battler);
}

static void PlayerHandleYesNoBox(enum BattlerId battler)
{
    if (IsOnPlayerSide(battler))
    {
        HandleBattleWindow(YESNOBOX_X_Y, 0);
        BattlePutTextOnWindow(gText_BattleYesNoChoice, B_WIN_YESNO);
        gMultiUsePlayerCursor = 1;
        BattleCreateYesNoCursorAt(1);
        gBattlerControllerFuncs[battler] = PlayerHandleYesNoInput;
    }
    else
    {
        BtlController_Complete(battler);
    }
}

void HandleChooseMoveAfterDma3(enum BattlerId battler)
{
    if (!IsDma3ManagerBusyWithBgCopy())
    {
        gBattle_BG0_X = 0;
        gBattle_BG0_Y = DISPLAY_HEIGHT * 2;
        gBattlerControllerFuncs[battler] = HandleInputChooseMove;
    }
}

// arenaMindPoints is used here as a placeholder for a timer.

static void PlayerChooseMoveInBattlePalace(enum BattlerId battler)
{
    if (--gBattleStruct->arenaMindPoints[battler] == 0)
    {
        gBattlePalaceMoveSelectionRngValue = gRngValue;
        BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, ChooseMoveAndTargetInBattlePalace(battler));
        BtlController_Complete(battler);
    }
}

void PlayerHandleChooseMove(enum BattlerId battler)
{
    if (gBattleTypeFlags & BATTLE_TYPE_PALACE)
    {
        gBattleStruct->arenaMindPoints[battler] = 8;
        gBattlerControllerFuncs[battler] = PlayerChooseMoveInBattlePalace;
    }
    else
    {
        struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);

        InitMoveSelectionsVarsAndStrings(battler);
        gBattleStruct->gimmick.playerSelect = FALSE;
        // The compact move panel replaces the vanilla L MOVE INFO prompt.

        AssignUsableZMoves(battler, moveInfo->moves);
        gBattleStruct->zmove.viable = (gBattleStruct->zmove.possibleZMoves[battler] & (1u << gMoveSelectionCursor[battler])) != 0;

        if (!IsGimmickTriggerSpriteActive())
            gBattleStruct->gimmick.triggerSpriteId = 0xFF;
        else if (!IsGimmickTriggerSpriteMatchingBattler(battler))
            DestroyGimmickTriggerSprite();
        if (!(gBattleStruct->gimmick.usableGimmick[battler] == GIMMICK_Z_MOVE && !gBattleStruct->zmove.viable))
            CreateGimmickTriggerSprite(battler);

        gBattlerControllerFuncs[battler] = HandleChooseMoveAfterDma3;
    }
}

void InitMoveSelectionsVarsAndStrings(enum BattlerId battler)
{
    DrawModernMoveSelectionPanels();
    LoadTypeIcons(battler);
    sUsingCompactMoveList[battler] = TRUE;
    DrawCompactMoveList(battler);
    gMultiUsePlayerCursor = GetOppositeBattler(battler);
    MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
    TryMoveSelectionDisplayMoveDescription(battler);
}

static void DrawCompactMoveList(enum BattlerId battler)
{
    static const u8 sCursor[] = _("{RIGHT_ARROW}");
    static const u8 sBlank[] = _(" ");
    static const u8 sPp[] = _("PP");
    s32 i;
    struct ChooseMoveStruct *moveInfo =
        (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);

    gNumberOfMovesToChoose = 0;
    FillWindowPixelBuffer(B_WIN_MOVE_NAME_1, PIXEL_FILL(0xE));

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        u8 *txtPtr;
        u8 y = i * 8;

        if (i == gMoveSelectionCursor[battler])
            StringCopy(gDisplayedStringBattle, sCursor);
        else
            StringCopy(gDisplayedStringBattle, sBlank);

        if (IsGimmickSelected(battler, GIMMICK_DYNAMAX) || GetActiveGimmick(battler) == GIMMICK_DYNAMAX)
            StringAppend(gDisplayedStringBattle, GetMoveName(GetMaxMove(battler, moveInfo->moves[i])));
        else
            StringAppend(gDisplayedStringBattle, GetMoveName(moveInfo->moves[i]));
        AddTextPrinterParameterized4(
            B_WIN_MOVE_NAME_1,
            FONT_COMPACT,
            0,
            y,
            0,
            0,
            sCompactMoveTextColors,
            TEXT_SKIP_DRAW,
            gDisplayedStringBattle);

        txtPtr = StringCopy(gDisplayedStringBattle, sPp);
        txtPtr = ConvertIntToDecimalStringN(
            txtPtr,
            moveInfo->currentPP[i],
            STR_CONV_MODE_RIGHT_ALIGN,
            2);
        *txtPtr++ = CHAR_SLASH;
        ConvertIntToDecimalStringN(
            txtPtr,
            moveInfo->maxPP[i],
            STR_CONV_MODE_RIGHT_ALIGN,
            2);

        AddTextPrinterParameterized4(
            B_WIN_MOVE_NAME_1,
            FONT_COMPACT,
            68,
            y,
            0,
            0,
            sCompactMoveTextColors,
            TEXT_SKIP_DRAW,
            gDisplayedStringBattle);

        if (moveInfo->moves[i] != MOVE_NONE)
            gNumberOfMovesToChoose++;
    }

    // Four 8px rows fill the 32px panel exactly; no post-draw scroll needed.
    PutWindowTilemap(B_WIN_MOVE_NAME_1);
    CopyWindowToVram(B_WIN_MOVE_NAME_1, COPYWIN_FULL);
}

static void DrawModernMoveSelectionPanels(void)
{
    FillBgTilemapBufferRect(0, 0, 0, 54, 30, 6, 0x11);
    HandleBattleWindow(0, 54, 15, 59, 0);
    HandleBattleWindow(16, 54, 29, 59, 0);
    CopyBgTilemapBufferToVram(0);
}

static void PlayerHandleChooseItem(enum BattlerId battler)
{
    s32 i;

    gBattlerInMenuId = battler;

    for (i = 0; i < ARRAY_COUNT(gBattlePartyCurrentOrder); i++)
        gBattlePartyCurrentOrder[i] = gBattleResources->bufferA[battler][1 + i];

#if MODULE_BATTLE_BAG_ENABLED
    if (sBattleBagItemPending[battler])
    {
        enum Item item = sBattleBagPendingItem[battler];

        sBattleBagItemPending[battler] = FALSE;
        sBattleBagPendingItem[battler] = ITEM_NONE;
        gSpecialVar_ItemId = item;

        if (!GetItemImportance(item)
         && !(B_TRY_CATCH_TRAINER_BALL >= GEN_4
           && GetItemBattleUsage(item) == EFFECT_ITEM_THROW_BALL
           && (gBattleTypeFlags & BATTLE_TYPE_TRAINER)))
            RemoveBagItem(item, 1);

        BtlController_EmitOneReturnValue(battler, B_COMM_TO_ENGINE, item);
        BtlController_Complete(battler);
        return;
    }
#endif

    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
    gBattlerControllerFuncs[battler] = OpenBagAndChooseItem;
}

static void PlayerHandleChoosePokemon(enum BattlerId battler)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(gBattlePartyCurrentOrder); i++)
        gBattlePartyCurrentOrder[i] = gBattleResources->bufferA[battler][4 + i];

    memcpy(gBattleStruct->battlerPartyOrders[battler], gBattlePartyCurrentOrder, sizeof(gBattlePartyCurrentOrder));

    if (gBattleTypeFlags & BATTLE_TYPE_ARENA && gBattleResources->bufferA[battler][1] != PARTY_ACTION_CANT_SWITCH
        && gBattleResources->bufferA[battler][1] != PARTY_ACTION_CHOOSE_FAINTED_MON
        && gBattleResources->bufferA[battler][1] != PARTY_ACTION_SEND_MON_TO_BOX)
    {
        BtlController_EmitChosenMonReturnValue(battler, B_COMM_TO_ENGINE, gBattlerPartyIndexes[battler] + 1, gBattlePartyCurrentOrder);
        BtlController_Complete(battler);
    }
    else
    {
        gBattleControllerData[battler] = CreateTask(TaskDummy, 0xFF);
        gTasks[gBattleControllerData[battler]].data[0] = gBattleResources->bufferA[battler][1];
        *(&gBattleStruct->battlerPreventingSwitchout) = gBattleResources->bufferA[battler][8];
        *(&gBattleStruct->prevSelectedPartySlot) = gBattleResources->bufferA[battler][2];
        *(&gBattleStruct->abilityPreventingSwitchout) = (gBattleResources->bufferA[battler][3] & 0xFF) | (gBattleResources->bufferA[battler][7] << 8);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
        gBattlerControllerFuncs[battler] = OpenPartyMenuToChooseMon;
        gBattlerInMenuId = battler;
    }
}

static void PlayerHandleCmd23(enum BattlerId battler)
{
    BattleStopLowHpSound();
    BeginNormalPaletteFade(PALETTES_ALL, 2, 0, 16, RGB_BLACK);
    BtlController_Complete(battler);
}

void PlayerHandleExpUpdate(enum BattlerId battler)
{
    u8 monId = gBattleResources->bufferA[battler][1];
    s32 taskId, expPointsToGive;

    if (GetMonData(&gParties[B_TRAINER_PLAYER][monId], MON_DATA_LEVEL) >= MAX_LEVEL)
    {
        BtlController_Complete(battler);
    }
    else
    {
        LoadBattleBarGfx(1);
        expPointsToGive = T1_READ_32(&gBattleResources->bufferA[battler][2]);
        taskId = CreateTask(Task_GiveExpToMon, 10);
        gTasks[taskId].tExpTask_monId = monId;
        gTasks[taskId].tExpTask_gainedExp_1 = expPointsToGive;
        gTasks[taskId].tExpTask_gainedExp_2 = expPointsToGive >> 16;
        gTasks[taskId].tExpTask_battler = battler;
        gBattlerControllerFuncs[battler] = BattleControllerDummy;
    }
}

#undef tExpTask_monId
#undef tExpTask_battler
#undef tExpTask_gainedExp_1
#undef tExpTask_gainedExp_2
#undef tExpTask_frames

static void PlayerHandleStatusXor(enum BattlerId battler)
{
    u32 val = GetMonData(GetBattlerMon(battler), MON_DATA_STATUS) ^ gBattleResources->bufferA[battler][1];

    SetMonData(GetBattlerMon(battler), MON_DATA_STATUS, &val);
    BtlController_Complete(battler);
}

static void PlayerHandleDMA3Transfer(enum BattlerId battler)
{
    u32 dstArg = gBattleResources->bufferA[battler][1]
            | (gBattleResources->bufferA[battler][2] << 8)
            | (gBattleResources->bufferA[battler][3] << 16)
            | (gBattleResources->bufferA[battler][4] << 24);
    u16 sizeArg = gBattleResources->bufferA[battler][5] | (gBattleResources->bufferA[battler][6] << 8);

    DmaCopyLarge16(3, &gBattleResources->bufferA[battler][7], (void *)dstArg, sizeArg, 0x1000);
    BtlController_Complete(battler);
}

static void PlayerHandlePlayBGM(enum BattlerId battler)
{
    PlayBGM(gBattleResources->bufferA[battler][1] | (gBattleResources->bufferA[battler][2] << 8));
    BtlController_Complete(battler);
}

static void PlayerHandleTwoReturnValues(enum BattlerId battler)
{
    BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_USE_MOVE, 0);
    BtlController_Complete(battler);
}

static void PlayerHandleChosenMonReturnValue(enum BattlerId battler)
{
    BtlController_EmitChosenMonReturnValue(battler, B_COMM_TO_ENGINE, 0, NULL);
    BtlController_Complete(battler);
}

static void PlayerHandleOneReturnValue(enum BattlerId battler)
{
    BtlController_EmitOneReturnValue(battler, B_COMM_TO_ENGINE, 0);
    BtlController_Complete(battler);
}

static void PlayerHandleOneReturnValue_Duplicate(enum BattlerId battler)
{
    BtlController_EmitOneReturnValue_Duplicate(battler, B_COMM_TO_ENGINE, 0);
    BtlController_Complete(battler);
}

static void PlayerHandleIntroTrainerBallThrow(enum BattlerId battler)
{
    enum TrainerPicID trainerPicID = PlayerGetTrainerBackPicId();
    const u16 *trainerPal = GetTrainerBackPicPalette(trainerPicID);
    BtlController_HandleIntroTrainerBallThrow(battler, 0xD6F8, trainerPal, 31, Intro_TryShinyAnimShowHealthbox);
}

static void PlayerHandleDrawPartyStatusSummary(enum BattlerId battler)
{
    BtlController_HandleDrawPartyStatusSummary(battler, B_SIDE_PLAYER, TRUE);
}

static void PlayerHandleEndBounceEffect(enum BattlerId battler)
{
    EndBounceEffect(battler, BOUNCE_HEALTHBOX);
    EndBounceEffect(battler, BOUNCE_MON);
    BtlController_Complete(battler);
}

static void PlayerHandleLinkStandbyMsg(enum BattlerId battler)
{
    RecordedBattle_RecordAllBattlerData(&gBattleResources->bufferA[battler][2]);
    switch (gBattleResources->bufferA[battler][1])
    {
    case LINK_STANDBY_MSG_STOP_BOUNCE:
        PrintLinkStandbyMsg();
        // fall through
    case LINK_STANDBY_STOP_BOUNCE_ONLY:
        EndBounceEffect(battler, BOUNCE_HEALTHBOX);
        EndBounceEffect(battler, BOUNCE_MON);
        break;
    case LINK_STANDBY_MSG_ONLY:
        PrintLinkStandbyMsg();
        break;
    }
    BtlController_Complete(battler);
}

static void PlayerHandleResetActionMoveSelection(enum BattlerId battler)
{
    switch (gBattleResources->bufferA[battler][1])
    {
    case RESET_ACTION_MOVE_SELECTION:
        gActionSelectionCursor[battler] = 0;
        gMoveSelectionCursor[battler] = 0;
        break;
    case RESET_ACTION_SELECTION:
        gActionSelectionCursor[battler] = 0;
        break;
    case RESET_MOVE_SELECTION:
        gMoveSelectionCursor[battler] = 0;
        break;
    }
    BtlController_Complete(battler);
}

static void PlayerHandleEndLinkBattle(enum BattlerId battler)
{
    RecordedBattle_RecordAllBattlerData(&gBattleResources->bufferA[battler][4]);
    gBattleOutcome = gBattleResources->bufferA[battler][1];
    gSaveBlock2Ptr->frontier.disableRecordBattle = gBattleResources->bufferA[battler][2];
    FadeOutMapMusic(5);
    BeginFastPaletteFade(3);
    BtlController_Complete(battler);
    gBattlerControllerFuncs[battler] = SetBattleEndCallbacks;
}

static void Controller_WaitForDebug(enum BattlerId battler)
{
    if (gMain.callback2 == BattleMainCB2 && !gPaletteFade.active)
    {
        BtlController_Complete(battler);
    }
}

static void PlayerHandleBattleDebug(enum BattlerId battler)
{
    BeginNormalPaletteFade(-1, 0, 0, 0x10, 0);
    SetMainCallback2(CB2_BattleDebugMenu);
    gBattlerControllerFuncs[battler] = Controller_WaitForDebug;
}

static bool32 ShouldShowTypeEffectiveness(u32 targetId)
{
    if (IsGhostBattleWithoutScope())
        return FALSE;

    if (B_SHOW_EFFECTIVENESS == SHOW_EFFECTIVENESS_CAUGHT)
        return GetSetPokedexFlag(SpeciesToNationalPokedexNum(gBattleMons[targetId].species), FLAG_GET_CAUGHT);

    if (B_SHOW_EFFECTIVENESS == SHOW_EFFECTIVENESS_SEEN)
        return GetSetPokedexFlag(SpeciesToNationalPokedexNum(gBattleMons[targetId].species), FLAG_GET_SEEN);

    return TRUE;
}

static u32 CheckTypeEffectiveness(enum BattlerId battlerAtk, enum BattlerId battlerDef)
{
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battlerAtk][4]);
    struct DamageContext ctx = {0};
    ctx.battlerAtk = battlerAtk;
    ctx.battlerDef = battlerDef;
    ctx.move = moveInfo->moves[gMoveSelectionCursor[battlerAtk]];
    ctx.moveType = CheckDynamicMoveType(GetBattlerMon(battlerAtk), ctx.move, battlerAtk, MON_IN_BATTLE);
    ctx.updateFlags = FALSE;
    ctx.abilities[ctx.battlerAtk] = GetBattlerAbility(battlerAtk);
    ctx.abilities[ctx.battlerDef] = GetBattlerAbility(battlerDef);
    ctx.holdEffects[ctx.battlerAtk] = GetBattlerHoldEffect(battlerAtk);
    ctx.holdEffects[ctx.battlerDef] = GetBattlerHoldEffect(battlerDef);

    uq4_12_t modifier = CalcTypeEffectivenessMultiplier(&ctx);

    if (!ShouldShowTypeEffectiveness(battlerDef))
        return EFFECTIVENESS_CANNOT_VIEW;

    if (modifier == UQ_4_12(0.0))
        return EFFECTIVENESS_NO_EFFECT; // No effect
    else if (modifier <= UQ_4_12(0.5))
        return EFFECTIVENESS_NOT_VERY_EFFECTIVE; // Not very effective
    else if (modifier >= UQ_4_12(2.0))
        return EFFECTIVENESS_SUPER_EFFECTIVE; // Super effective
    return EFFECTIVENESS_NORMAL; // Normal effectiveness
}

static u32 CheckTargetTypeEffectiveness(enum BattlerId battler)
{
    enum BattlerId battlerFoe = GetOppositeBattler(battler);
    u32 foeEffectiveness = CheckTypeEffectiveness(battler, battlerFoe);

    if (IsDoubleBattle())
    {
        enum BattlerId partnerFoe = GetPartnerBattler(battlerFoe);
        u32 partnerFoeEffectiveness = CheckTypeEffectiveness(battler, partnerFoe);
        if (!IsBattlerAlive(battlerFoe))
            return partnerFoeEffectiveness;
        if (IsBattlerAlive(battlerFoe) && IsBattlerAlive(partnerFoe)
         && partnerFoeEffectiveness > foeEffectiveness)
            return partnerFoeEffectiveness;
    }
    return foeEffectiveness; // fallthrough for any other circumstance
}

// The compact panel always previews the battle's type chart. This is intentionally
// independent from the global seen/caught display setting used by the vanilla UI.
static u32 CheckCompactTypeEffectiveness(enum BattlerId battlerAtk, enum BattlerId battlerDef)
{
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battlerAtk][4]);
    struct DamageContext ctx = {0};
    uq4_12_t modifier;

    ctx.battlerAtk = battlerAtk;
    ctx.battlerDef = battlerDef;
    ctx.move = moveInfo->moves[gMoveSelectionCursor[battlerAtk]];
    ctx.moveType = CheckDynamicMoveType(GetBattlerMon(battlerAtk), ctx.move, battlerAtk, MON_IN_BATTLE);
    ctx.updateFlags = FALSE;
    ctx.abilities[ctx.battlerAtk] = GetBattlerAbility(battlerAtk);
    ctx.abilities[ctx.battlerDef] = GetBattlerAbility(battlerDef);
    ctx.holdEffects[ctx.battlerAtk] = GetBattlerHoldEffect(battlerAtk);
    ctx.holdEffects[ctx.battlerDef] = GetBattlerHoldEffect(battlerDef);
    modifier = CalcTypeEffectivenessMultiplier(&ctx);

    if (modifier == UQ_4_12(0.0))
        return EFFECTIVENESS_NO_EFFECT;
    if (modifier <= UQ_4_12(0.5))
        return EFFECTIVENESS_NOT_VERY_EFFECTIVE;
    if (modifier >= UQ_4_12(2.0))
        return EFFECTIVENESS_SUPER_EFFECTIVE;
    return EFFECTIVENESS_NORMAL;
}

static u32 CheckCompactTargetTypeEffectiveness(enum BattlerId battler)
{
    enum BattlerId battlerFoe = GetOppositeBattler(battler);
    u32 foeEffectiveness = CheckCompactTypeEffectiveness(battler, battlerFoe);

    if (IsDoubleBattle())
    {
        enum BattlerId partnerFoe = GetPartnerBattler(battlerFoe);
        u32 partnerFoeEffectiveness = CheckCompactTypeEffectiveness(battler, partnerFoe);

        if (!IsBattlerAlive(battlerFoe))
            return partnerFoeEffectiveness;
        if (IsBattlerAlive(battlerFoe) && IsBattlerAlive(partnerFoe)
         && partnerFoeEffectiveness > foeEffectiveness)
            return partnerFoeEffectiveness;
    }
    return foeEffectiveness;
}

static void MoveSelectionDisplayMoveEffectiveness(u32 foeEffectiveness, enum BattlerId battler)
{
    static const u8 noIcon[] =  _("");
    static const u8 effectiveIcon[] =  _("{CIRCLE_HOLLOW}");
    static const u8 superEffectiveIcon[] =  _("{CIRCLE_DOT}");
    static const u8 notVeryEffectiveIcon[] =  _("{TRIANGLE}");
    static const u8 immuneIcon[] =  _("{BIG_MULT_X}");
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
    u8 *txtPtr;

    if (sUsingCompactMoveList[battler])
        return;

    txtPtr = StringCopy(gDisplayedStringBattle, gText_MoveInterfacePP);

    if (!IsBattleMoveStatus(moveInfo->moves[gMoveSelectionCursor[battler]]))
    {
        switch (foeEffectiveness)
        {
        case EFFECTIVENESS_SUPER_EFFECTIVE:
            StringCopy(txtPtr, superEffectiveIcon);
            break;
        case EFFECTIVENESS_NOT_VERY_EFFECTIVE:
            StringCopy(txtPtr, notVeryEffectiveIcon);
            break;
        case EFFECTIVENESS_NO_EFFECT:
            StringCopy(txtPtr, immuneIcon);
            break;
        case EFFECTIVENESS_NORMAL:
            StringCopy(txtPtr, effectiveIcon);
            break;
        default:
        case EFFECTIVENESS_CANNOT_VIEW:
            StringCopy(txtPtr, noIcon);
            break;
        }
    }

    BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_PP);
}
