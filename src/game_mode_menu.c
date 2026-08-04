#include "global.h"
#include "bg.h"
#include "decompress.h"
#include "dma3.h"
#include "game_mode.h"
#include "game_mode_menu.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "main_menu.h"
#include "menu.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

#define tSelection data[0]

enum
{
    WIN_HEADER,
    WIN_BODY,
    WIN_FOOTER,
};

static EWRAM_DATA MainCallback sBackCallback = NULL;

static void CB2_InitGameModeMenu(void);
static void MainCB2(void);
static void VBlankCB(void);
static void Task_FadeIn(u8 taskId);
static void Task_ProcessInput(u8 taskId);
static void Task_FadeOutStart(u8 taskId);
static void Task_FadeOutBack(u8 taskId);
static void DrawMenu(u8 taskId);

static const u8 sTextHeader[] = _("CHOOSE YOUR ADVENTURE");
static const u8 sTextVanilla[] = _("VANILLA");
static const u8 sTextNuzlocke[] = _("NUZLOCKE");
static const u8 sTextCarnage[] = _("CARNAGE");
static const u8 sTagVanilla[] = _("CLASSIC");
static const u8 sTagNuzlocke[] = _("CHALLENGE");
static const u8 sTagCarnage[] = _("CHAOS");
static const u8 sHeadlineVanilla[] = _("THE ORIGINAL\nJOURNEY");
static const u8 sHeadlineNuzlocke[] = _("EVERY CHOICE\nMATTERS");
static const u8 sHeadlineCarnage[] = _("EMBRACE THE\nUNEXPECTED");
static const u8 sDescVanilla[] = _("Classic rules.\nRandomizer optional.");
static const u8 sDescNuzlocke[] = _("Custom high-stakes\nNuzlocke rules.");
static const u8 sDescCarnage[] = _("Randomizers and\nlevel chaos stay on.");
static const u8 sTextControls[] = _("{DPAD_UPDOWN} MOVE     {A_BUTTON} SELECT     {B_BUTTON} BACK");

static const u8 sTextColorsNormal[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sTextColorsSelected[] = {TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY};
static const u8 sTextColorsDesc[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};

static const u8 *const sModeNames[GAME_MODE_COUNT] =
{
    [GAME_MODE_VANILLA] = sTextVanilla,
    [GAME_MODE_NUZLOCKE] = sTextNuzlocke,
    [GAME_MODE_CARNAGE] = sTextCarnage,
};

static const u8 *const sModeDescriptions[GAME_MODE_COUNT] =
{
    [GAME_MODE_VANILLA] = sDescVanilla,
    [GAME_MODE_NUZLOCKE] = sDescNuzlocke,
    [GAME_MODE_CARNAGE] = sDescCarnage,
};

static const u8 *const sModeHeadlines[GAME_MODE_COUNT] =
{
    [GAME_MODE_VANILLA] = sHeadlineVanilla,
    [GAME_MODE_NUZLOCKE] = sHeadlineNuzlocke,
    [GAME_MODE_CARNAGE] = sHeadlineCarnage,
};

static const u8 *const sModeTags[GAME_MODE_COUNT] =
{
    [GAME_MODE_VANILLA] = sTagVanilla,
    [GAME_MODE_NUZLOCKE] = sTagNuzlocke,
    [GAME_MODE_CARNAGE] = sTagCarnage,
};

static const struct WindowTemplate sWindows[] =
{
    [WIN_HEADER] = { .bg = 1, .tilemapLeft = 2, .tilemapTop = 1, .width = 26, .height = 2, .paletteNum = 1, .baseBlock = 2 },
    [WIN_BODY] = { .bg = 0, .tilemapLeft = 2, .tilemapTop = 5, .width = 26, .height = 10, .paletteNum = 1, .baseBlock = 0x36 },
    [WIN_FOOTER] = { .bg = 0, .tilemapLeft = 2, .tilemapTop = 17, .width = 26, .height = 2, .paletteNum = 1, .baseBlock = 0x13A },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sBgs[] =
{
    { .bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0 },
    { .bg = 1, .charBaseIndex = 0, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 1, .baseTile = 0 },
};

static const u16 sBgPalette[] = {RGB(17, 18, 31)};
static const u16 sTextPalette[] = INCGFX_U16("graphics/interface/option_menu_text.pal", ".gbapal");

void StartGameModeMenu(MainCallback backCallback)
{
    sBackCallback = backCallback;
    gMain.state = 0;
    SetMainCallback2(CB2_InitGameModeMenu);
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

static void CB2_InitGameModeMenu(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        gMain.state++;
        break;
    case 1:
        DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgs, ARRAY_COUNT(sBgs));
        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
        InitWindows(sWindows);
        DeactivateAllTextPrinters();
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_CLR);
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_DARKEN);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 4);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
        gMain.state++;
        break;
    case 2:
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        gMain.state++;
        break;
    case 3:
        LoadBgTiles(1, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1A2);
        LoadPalette(sBgPalette, BG_PLTT_ID(0), sizeof(sBgPalette));
        LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        LoadPalette(sTextPalette, BG_PLTT_ID(1), sizeof(sTextPalette));
        gMain.state++;
        break;
    case 4:
        PutWindowTilemap(WIN_HEADER);
        PutWindowTilemap(WIN_BODY);
        PutWindowTilemap(WIN_FOOTER);
        FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
        AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, sTextHeader,
                                    GetStringCenterAlignXOffset(FONT_NORMAL, sTextHeader, 208), 1,
                                    TEXT_SKIP_DRAW, NULL);
        FillWindowPixelBuffer(WIN_FOOTER, PIXEL_FILL(1));
        AddTextPrinterParameterized(WIN_FOOTER, FONT_SMALL, sTextControls,
                                    GetStringCenterAlignXOffset(FONT_SMALL, sTextControls, 208), 0,
                                    TEXT_SKIP_DRAW, NULL);
        DrawTextBorderOuter(WIN_HEADER, 0x1A2, 7);
        DrawTextBorderOuter(WIN_BODY, 0x1A2, 7);
        DrawTextBorderOuter(WIN_FOOTER, 0x1A2, 7);
        CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
        CopyWindowToVram(WIN_FOOTER, COPYWIN_FULL);
        CopyBgTilemapBufferToVram(0);
        CopyBgTilemapBufferToVram(1);
        gMain.state++;
        break;
    case 5:
    {
        u8 taskId = CreateTask(Task_FadeIn, 0);
        gTasks[taskId].tSelection = GAME_MODE_VANILLA;
        DrawMenu(taskId);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB);
        SetMainCallback2(MainCB2);
        break;
    }
    }
}

static void DrawMenu(u8 taskId)
{
    u32 i;
    u8 selection = gTasks[taskId].tSelection;

    FillWindowPixelBuffer(WIN_BODY, PIXEL_FILL(1));
    FillWindowPixelRect(WIN_BODY, PIXEL_FILL(2), 96, 4, 1, 72);
    for (i = 0; i < GAME_MODE_COUNT; i++)
    {
        static const u8 sCursor[] = _("▶");
        const u8 *colors = i == selection ? sTextColorsSelected : sTextColorsNormal;
        u8 y = i * 24 + 5;

        if (i == selection)
        {
            FillWindowPixelRect(WIN_BODY, PIXEL_FILL(2), 0, i * 24 + 2, 94, 20);
            AddTextPrinterParameterized3(WIN_BODY, FONT_NORMAL, 4, y, colors, TEXT_SKIP_DRAW, sCursor);
        }
        AddTextPrinterParameterized3(WIN_BODY, FONT_NORMAL, 20, y, colors, TEXT_SKIP_DRAW, sModeNames[i]);
    }
    AddTextPrinterParameterized3(WIN_BODY, FONT_SMALL, 104, 3, sTextColorsDesc,
                                 TEXT_SKIP_DRAW, sModeTags[selection]);
    FillWindowPixelRect(WIN_BODY, PIXEL_FILL(2), 104, 16, 96, 1);
    AddTextPrinterParameterized3(WIN_BODY, FONT_SMALL, 104, 20, sTextColorsDesc,
                                 TEXT_SKIP_DRAW, sModeHeadlines[selection]);
    AddTextPrinterParameterized3(WIN_BODY, FONT_SMALL, 104, 50, sTextColorsDesc,
                                 TEXT_SKIP_DRAW, sModeDescriptions[selection]);
    CopyWindowToVram(WIN_BODY, COPYWIN_FULL);
}

static void Task_FadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_ProcessInput;
}

static void Task_ProcessInput(u8 taskId)
{
    if (JOY_NEW(DPAD_UP))
    {
        gTasks[taskId].tSelection = gTasks[taskId].tSelection == 0 ? GAME_MODE_COUNT - 1 : gTasks[taskId].tSelection - 1;
        DrawMenu(taskId);
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        gTasks[taskId].tSelection = (gTasks[taskId].tSelection + 1) % GAME_MODE_COUNT;
        DrawMenu(taskId);
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        GameMode_SetPending(gTasks[taskId].tSelection);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_FadeOutStart;
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_FadeOutBack;
        PlaySE(SE_SELECT);
    }
}

static void FinishMenu(u8 taskId, MainCallback callback)
{
    DestroyTask(taskId);
    FreeAllWindowBuffers();
    gMain.state = 0;
    SetMainCallback2(callback);
}

static void Task_FadeOutStart(u8 taskId)
{
    if (!gPaletteFade.active)
        FinishMenu(taskId, CB2_StartSelectedNewGame);
}

static void Task_FadeOutBack(u8 taskId)
{
    if (!gPaletteFade.active)
        FinishMenu(taskId, sBackCallback);
}
