#include "global.h"
#include "bg.h"
#include "decompress.h"
#include "dma3.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "mail.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "scanline_effect.h"
#include "shared_transfer_box.h"
#include "shared_transfer_box_menu.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

#define VISIBLE_TRANSFER_ROWS 6
#define tSlot data[0]
#define tParty data[1]

enum
{
    WIN_HEADER,
    WIN_BODY,
};

static EWRAM_DATA struct SharedTransferBox *sTransferBox = NULL;
static EWRAM_DATA const u8 *sStatusText = NULL;
static EWRAM_DATA bool8 sOpenedFromParty = FALSE;

static void CB2_InitTransferBox(void);
static void MainCB2(void);
static void VBlankCB(void);
static void Task_FadeIn(u8 taskId);
static void Task_Input(u8 taskId);
static void Task_FadeOut(u8 taskId);
static void DrawScreen(u8 taskId);
static bool32 DepositSelectedPartyMon(u8 taskId);
static bool32 WithdrawSelectedMon(u8 taskId);

static const u8 sTextHeader[] = _("SHARED TRANSFER BOX");
static const u8 sTextCursor[] = _(">");
static const u8 sTextEmpty[] = _("-- EMPTY --");
static const u8 sTextHelp[] = _("{DPAD_UPDOWN} SLOT\n{DPAD_LEFTRIGHT} PARTY\n{A_BUTTON} MOVE\n{B_BUTTON} EXIT");
static const u8 sTextParty[] = _("PARTY");
static const u8 sTextShared[] = _("SHARED  /20");
static const u8 sTextStored[] = _("POKéMON\nDEPOSITED.\nTRANSFER SAVED.");
static const u8 sTextTaken[] = _("POKéMON\nWITHDRAWN.\nTRANSFER SAVED.");
static const u8 sTextLastMon[] = _("KEEP ONE\nPOKéMON IN\nYOUR PARTY.");
static const u8 sTextPartyFull[] = _("YOUR PARTY\nIS FULL.");
static const u8 sTextMail[] = _("REMOVE MAIL\nFIRST.");
static const u8 sTextSaveFailed[] = _("SAVE FAILED.\nTRANSFER\nCANCELLED.");
static const u8 sTextSlash[] = _("/");

static const struct WindowTemplate sWindows[] =
{
    [WIN_HEADER] = { .bg = 1, .tilemapLeft = 2, .tilemapTop = 1, .width = 26, .height = 2, .paletteNum = 1, .baseBlock = 2 },
    [WIN_BODY] = { .bg = 0, .tilemapLeft = 1, .tilemapTop = 5, .width = 28, .height = 13, .paletteNum = 1, .baseBlock = 0x36 },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sBgs[] =
{
    { .bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0 },
    { .bg = 1, .charBaseIndex = 0, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 1, .baseTile = 0 },
};

static const u16 sBgPalette[] = {RGB(17, 18, 31)};
static const u16 sTextPalette[] = INCGFX_U16("graphics/interface/option_menu_text.pal", ".gbapal");

void StartSharedTransferBoxMenu(void)
{
    sOpenedFromParty = FALSE;
    gMain.state = 0;
    SetMainCallback2(CB2_InitTransferBox);
}

void StartSharedTransferBoxMenuFromParty(void)
{
    sOpenedFromParty = TRUE;
    gMain.state = 0;
    SetMainCallback2(CB2_InitTransferBox);
}

static void MainCB2(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_InitTransferBox(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgs, ARRAY_COUNT(sBgs));
        InitWindows(sWindows);
        DeactivateAllTextPrinters();
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        gMain.state++;
        break;
    case 1:
        LoadBgTiles(1, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1A2);
        LoadPalette(sBgPalette, BG_PLTT_ID(0), sizeof(sBgPalette));
        LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        LoadPalette(sTextPalette, BG_PLTT_ID(1), sizeof(sTextPalette));
        sTransferBox = AllocZeroed(sizeof(*sTransferBox));
        if (sTransferBox != NULL)
            SharedTransferBox_Load(sTransferBox);
        gMain.state++;
        break;
    case 2:
    {
        u8 taskId;
        u8 i;
        for (i = 0; i < ARRAY_COUNT(sWindows) - 1; i++)
        {
            PutWindowTilemap(i);
            FillWindowPixelBuffer(i, PIXEL_FILL(1));
            DrawTextBorderOuter(i, 0x1A2, 7);
        }
        taskId = CreateTask(Task_FadeIn, 0);
        DrawScreen(taskId);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
        SetVBlankCallback(VBlankCB);
        SetMainCallback2(MainCB2);
        break;
    }
    }
}

static void DrawScreen(u8 taskId)
{
    u8 first = (gTasks[taskId].tSlot / VISIBLE_TRANSFER_ROWS) * VISIBLE_TRANSFER_ROWS;
    u8 partyCount = CalculatePlayerPartyCount();
    u8 i;

    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, sTextHeader,
                                GetStringCenterAlignXOffset(FONT_NORMAL, sTextHeader, 208), 1, TEXT_SKIP_DRAW, NULL);
    FillWindowPixelBuffer(WIN_BODY, PIXEL_FILL(1));
    FillWindowPixelRect(WIN_BODY, PIXEL_FILL(2), 144, 0, 1, 104);
    FillWindowPixelRect(WIN_BODY, PIXEL_FILL(2), 145, 52, 79, 1);
    for (i = 0; i < VISIBLE_TRANSFER_ROWS && first + i < SHARED_TRANSFER_BOX_CAPACITY; i++)
    {
        u8 slot = first + i;
        u8 y = i * 16 + 1;
        ConvertIntToDecimalStringN(gStringVar1, slot + 1, STR_CONV_MODE_LEADING_ZEROS, 2);
        AddTextPrinterParameterized(WIN_BODY, FONT_SMALL, gStringVar1, 4, y + 2, TEXT_SKIP_DRAW, NULL);
        if (slot == gTasks[taskId].tSlot)
            AddTextPrinterParameterized(WIN_BODY, FONT_NORMAL, sTextCursor, 22, y, TEXT_SKIP_DRAW, NULL);
        if (sTransferBox != NULL && SharedTransferBox_IsSlotOccupied(sTransferBox, slot))
        {
            struct BoxPokemon *mon = &sTransferBox->mons[slot];
            GetBoxMonData(mon, MON_DATA_NICKNAME, gStringVar2);
            AddTextPrinterParameterized(WIN_BODY, FONT_NORMAL, gStringVar2, 34, y, TEXT_SKIP_DRAW, NULL);
            ConvertIntToDecimalStringN(gStringVar3, GetLevelFromBoxMonExp(mon), STR_CONV_MODE_LEFT_ALIGN, 3);
            AddTextPrinterParameterized(WIN_BODY, FONT_SMALL, gStringVar3, 116, y + 2, TEXT_SKIP_DRAW, NULL);
        }
        else
            AddTextPrinterParameterized(WIN_BODY, FONT_SMALL, sTextEmpty, 36, y + 2, TEXT_SKIP_DRAW, NULL);
    }

    AddTextPrinterParameterized(WIN_BODY, FONT_SMALL, sTextParty, 151, 1, TEXT_SKIP_DRAW, NULL);
    if (partyCount != 0)
    {
        if (gTasks[taskId].tParty >= partyCount)
            gTasks[taskId].tParty = partyCount - 1;
        GetMonData(&gParties[B_TRAINER_PLAYER][gTasks[taskId].tParty], MON_DATA_NICKNAME, gStringVar1);
        AddTextPrinterParameterized(WIN_BODY, FONT_NORMAL, gStringVar1, 151, 16, TEXT_SKIP_DRAW, NULL);
        ConvertIntToDecimalStringN(gStringVar2, gTasks[taskId].tParty + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
        AddTextPrinterParameterized(WIN_BODY, FONT_SMALL, gStringVar2, 190, 1, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(WIN_BODY, FONT_SMALL, sTextSlash, 200, 1, TEXT_SKIP_DRAW, NULL);
        ConvertIntToDecimalStringN(gStringVar3, partyCount, STR_CONV_MODE_LEFT_ALIGN, 1);
        AddTextPrinterParameterized(WIN_BODY, FONT_SMALL, gStringVar3, 210, 1, TEXT_SKIP_DRAW, NULL);
    }

    if (sStatusText == NULL)
    {
        ConvertIntToDecimalStringN(gStringVar1, sTransferBox != NULL ? SharedTransferBox_Count(sTransferBox) : 0,
                                   STR_CONV_MODE_LEFT_ALIGN, 2);
        AddTextPrinterParameterized(WIN_BODY, FONT_SMALL, sTextShared, 151, 35, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(WIN_BODY, FONT_SMALL, gStringVar1, 190, 35, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(WIN_BODY, FONT_SMALL, sTextHelp, 151, 55, TEXT_SKIP_DRAW, NULL);
    }
    else
    {
        AddTextPrinterParameterized(WIN_BODY, FONT_SMALL, sStatusText, 151, 58, TEXT_SKIP_DRAW, NULL);
    }
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
    CopyWindowToVram(WIN_BODY, COPYWIN_FULL);
}

static void Task_FadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_Input;
}

static void Task_Input(u8 taskId)
{
    u8 partyCount = CalculatePlayerPartyCount();

    if (sStatusText != NULL && JOY_NEW(A_BUTTON | B_BUTTON))
    {
        sStatusText = NULL;
        DrawScreen(taskId);
    }
    else if (JOY_NEW(DPAD_UP))
    {
        gTasks[taskId].tSlot = gTasks[taskId].tSlot == 0 ? SHARED_TRANSFER_BOX_CAPACITY - 1 : gTasks[taskId].tSlot - 1;
        DrawScreen(taskId);
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        gTasks[taskId].tSlot = (gTasks[taskId].tSlot + 1) % SHARED_TRANSFER_BOX_CAPACITY;
        DrawScreen(taskId);
        PlaySE(SE_SELECT);
    }
    else if (partyCount != 0 && JOY_NEW(DPAD_LEFT))
    {
        gTasks[taskId].tParty = gTasks[taskId].tParty == 0 ? partyCount - 1 : gTasks[taskId].tParty - 1;
        DrawScreen(taskId);
        PlaySE(SE_SELECT);
    }
    else if (partyCount != 0 && JOY_NEW(DPAD_RIGHT))
    {
        gTasks[taskId].tParty = (gTasks[taskId].tParty + 1) % partyCount;
        DrawScreen(taskId);
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(A_BUTTON) && sTransferBox != NULL)
    {
        if (SharedTransferBox_IsSlotOccupied(sTransferBox, gTasks[taskId].tSlot))
            WithdrawSelectedMon(taskId);
        else
            DepositSelectedPartyMon(taskId);
        DrawScreen(taskId);
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_FadeOut;
        PlaySE(SE_SELECT);
    }
}

static bool32 DepositSelectedPartyMon(u8 taskId)
{
    struct Pokemon partyBackup[PARTY_SIZE];
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gTasks[taskId].tParty];
    u8 slot = gTasks[taskId].tSlot;

    if (CalculatePlayerPartyCount() <= 1)
    {
        sStatusText = sTextLastMon;
        return FALSE;
    }
    if (ItemIsMail(GetMonData(mon, MON_DATA_HELD_ITEM)))
    {
        sStatusText = sTextMail;
        return FALSE;
    }
    memcpy(partyBackup, gParties[B_TRAINER_PLAYER], sizeof(partyBackup));
    if (!SharedTransferBox_Deposit(sTransferBox, slot, &mon->box) || !SharedTransferBox_Save(sTransferBox))
    {
        sStatusText = sTextSaveFailed;
        return FALSE;
    }
    ZeroMonData(mon);
    CompactPartySlots();
    CalculatePlayerPartyCount();
    if (TrySavingData(SAVE_NORMAL) != SAVE_STATUS_OK)
    {
        memcpy(gParties[B_TRAINER_PLAYER], partyBackup, sizeof(partyBackup));
        CalculatePlayerPartyCount();
        ZeroBoxMonData(&sTransferBox->mons[slot]);
        SharedTransferBox_Save(sTransferBox);
        sStatusText = sTextSaveFailed;
        return FALSE;
    }
    sStatusText = sTextStored;
    return TRUE;
}

static bool32 WithdrawSelectedMon(u8 taskId)
{
    struct Pokemon partyBackup[PARTY_SIZE];
    struct BoxPokemon mon;
    u8 partyCount = CalculatePlayerPartyCount();
    u8 slot = gTasks[taskId].tSlot;

    if (partyCount >= PARTY_SIZE)
    {
        sStatusText = sTextPartyFull;
        return FALSE;
    }
    memcpy(partyBackup, gParties[B_TRAINER_PLAYER], sizeof(partyBackup));
    if (!SharedTransferBox_Withdraw(sTransferBox, slot, &mon) || !SharedTransferBox_Save(sTransferBox))
    {
        sStatusText = sTextSaveFailed;
        return FALSE;
    }
    BoxMonToMon(&mon, &gParties[B_TRAINER_PLAYER][partyCount]);
    CalculatePlayerPartyCount();
    if (TrySavingData(SAVE_NORMAL) != SAVE_STATUS_OK)
    {
        memcpy(gParties[B_TRAINER_PLAYER], partyBackup, sizeof(partyBackup));
        CalculatePlayerPartyCount();
        SharedTransferBox_Deposit(sTransferBox, slot, &mon);
        SharedTransferBox_Save(sTransferBox);
        sStatusText = sTextSaveFailed;
        return FALSE;
    }
    sStatusText = sTextTaken;
    return TRUE;
}

static void Task_FadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        Free(sTransferBox);
        sTransferBox = NULL;
        sStatusText = NULL;
        FreeAllWindowBuffers();
        if (sOpenedFromParty)
        {
            sOpenedFromParty = FALSE;
            SetMainCallback2(CB2_ReturnToFieldWithOpenMenu);
        }
        else
        {
            gFieldCallback = ShowPokemonStorageSystemPC;
            SetMainCallback2(CB2_ReturnToField);
        }
    }
}

#undef tSlot
#undef tParty
