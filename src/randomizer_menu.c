#include "global.h"
#include "randomizer_menu.h"
#include "bg.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "randomizer.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

#define tMenuSelection data[0]

static EWRAM_DATA MainCallback sBackCallback = NULL;

enum
{
    MENUITEM_WILD,
    MENUITEM_STARTERS,
    MENUITEM_SEED,
    MENUITEM_REROLL,
    MENUITEM_START_GAME,
    MENUITEM_COUNT,
};

enum
{
    WIN_HEADER,
    WIN_OPTIONS,
};

static void CB2_InitRandomizerSetupMenu(void);
static void MainCB2(void);
static void VBlankCB(void);
static void Task_FadeIn(u8 taskId);
static void Task_ProcessInput(u8 taskId);
static void Task_FadeOut(u8 taskId);
static void DrawHeader(void);
static void DrawMenu(void);
static void DrawToggle(u8 menuItem, bool8 enabled);
static void DrawSeed(void);
static void HighlightMenuItem(u8 menuItem);
static void DrawBgWindowFrames(void);

static const u8 sText_Header[] = _("RANDOMIZER SETUP");
static const u8 sText_On[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ON");
static const u8 sText_Off[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}OFF");
static const u8 *const sMenuItemNames[MENUITEM_COUNT] =
{
    [MENUITEM_WILD]       = COMPOUND_STRING("WILD POKéMON"),
    [MENUITEM_STARTERS]   = COMPOUND_STRING("STARTERS"),
    [MENUITEM_SEED]       = COMPOUND_STRING("SEED"),
    [MENUITEM_REROLL]     = COMPOUND_STRING("REROLL SEED"),
    [MENUITEM_START_GAME] = COMPOUND_STRING("START GAME"),
};

static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_HEADER] = {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 1,
        .width = 26,
        .height = 2,
        .paletteNum = 1,
        .baseBlock = 2,
    },
    [WIN_OPTIONS] = {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 5,
        .width = 26,
        .height = 10,
        .paletteNum = 1,
        .baseBlock = 0x36,
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0,
    },
    {
        .bg = 0,
        .charBaseIndex = 1,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0,
    },
};

static const u16 sBgPalette[] = {RGB(17, 18, 31)};
static const u16 sTextPalette[] = INCGFX_U16("graphics/interface/option_menu_text.pal", ".gbapal");

void StartRandomizerSetupMenu(MainCallback returnCallback)
{
    sBackCallback = NULL;
    gMain.savedCallback = returnCallback;
    gMain.state = 0;
    SetMainCallback2(CB2_InitRandomizerSetupMenu);
}

void StartRandomizerSetupMenuWithBack(MainCallback returnCallback, MainCallback backCallback)
{
    sBackCallback = backCallback;
    gMain.savedCallback = returnCallback;
    gMain.state = 0;
    SetMainCallback2(CB2_InitRandomizerSetupMenu);
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

static void CB2_InitRandomizerSetupMenu(void)
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
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
        InitWindows(sWindowTemplates);
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
        PutWindowTilemap(WIN_OPTIONS);
        DrawHeader();
        DrawMenu();
        DrawBgWindowFrames();
        gMain.state++;
        break;
    case 5:
    {
        u8 taskId = CreateTask(Task_FadeIn, 0);

        gTasks[taskId].tMenuSelection = MENUITEM_WILD;
        HighlightMenuItem(MENUITEM_WILD);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB);
        SetMainCallback2(MainCB2);
        break;
    }
    }
}

static void Task_FadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_ProcessInput;
}

static void Task_ProcessInput(u8 taskId)
{
    u8 selection = gTasks[taskId].tMenuSelection;

    if (JOY_NEW(DPAD_UP))
    {
        selection = (selection == 0) ? MENUITEM_START_GAME : selection - 1;
        gTasks[taskId].tMenuSelection = selection;
        HighlightMenuItem(selection);
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        selection = (selection == MENUITEM_START_GAME) ? 0 : selection + 1;
        gTasks[taskId].tMenuSelection = selection;
        HighlightMenuItem(selection);
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT)
          && (selection == MENUITEM_WILD || selection == MENUITEM_STARTERS))
    {
        if (selection == MENUITEM_WILD)
        {
            Randomizer_SetWildEnabled(!Randomizer_IsWildEnabled());
            DrawToggle(MENUITEM_WILD, Randomizer_IsWildEnabled());
        }
        else
        {
            Randomizer_SetStarterEnabled(!Randomizer_IsStarterEnabled());
            DrawToggle(MENUITEM_STARTERS, Randomizer_IsStarterEnabled());
        }
        CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        switch (selection)
        {
        case MENUITEM_WILD:
            Randomizer_SetWildEnabled(!Randomizer_IsWildEnabled());
            DrawToggle(MENUITEM_WILD, Randomizer_IsWildEnabled());
            CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
            PlaySE(SE_SELECT);
            break;
        case MENUITEM_STARTERS:
            Randomizer_SetStarterEnabled(!Randomizer_IsStarterEnabled());
            DrawToggle(MENUITEM_STARTERS, Randomizer_IsStarterEnabled());
            CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
            PlaySE(SE_SELECT);
            break;
        case MENUITEM_REROLL:
            Randomizer_RerollSeed();
            DrawSeed();
            CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
            PlaySE(SE_SELECT);
            break;
        case MENUITEM_START_GAME:
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            gTasks[taskId].func = Task_FadeOut;
            PlaySE(SE_SELECT);
            break;
        }
    }
    else if (JOY_NEW(B_BUTTON) && sBackCallback != NULL)
    {
        gMain.savedCallback = sBackCallback;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_FadeOut;
        PlaySE(SE_SELECT);
    }
}

static void Task_FadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        MainCallback returnCallback = gMain.savedCallback;

        DestroyTask(taskId);
        FreeAllWindowBuffers();
        gMain.state = 0;
        SetMainCallback2(returnCallback);
    }
}

static void DrawHeader(void)
{
    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, sText_Header, 8, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void DrawMenu(void)
{
    u8 i;

    FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
    for (i = 0; i < MENUITEM_COUNT; i++)
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, sMenuItemNames[i], 8, i * 16 + 1, TEXT_SKIP_DRAW, NULL);

    DrawToggle(MENUITEM_WILD, Randomizer_IsWildEnabled());
    DrawToggle(MENUITEM_STARTERS, Randomizer_IsStarterEnabled());
    DrawSeed();
    CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
}

static void DrawToggle(u8 menuItem, bool8 enabled)
{
    const u8 *text = enabled ? sText_On : sText_Off;
    u8 y = menuItem * 16 + 1;

    FillWindowPixelRect(WIN_OPTIONS, PIXEL_FILL(1), 176, y, 24, 14);
    AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, text,
                                GetStringRightAlignXOffset(FONT_NORMAL, text, 200),
                                y, TEXT_SKIP_DRAW, NULL);
}

static void DrawSeed(void)
{
    u8 text[9];
    u8 y = MENUITEM_SEED * 16 + 1;
    u32 seed = Randomizer_GetSeed();
    u8 *end;

    end = ConvertIntToHexStringN(text, seed >> 16, STR_CONV_MODE_LEADING_ZEROS, 4);
    ConvertIntToHexStringN(end, seed & 0xFFFF, STR_CONV_MODE_LEADING_ZEROS, 4);
    FillWindowPixelRect(WIN_OPTIONS, PIXEL_FILL(1), 128, y, 72, 14);
    AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, text,
                                GetStringRightAlignXOffset(FONT_NORMAL, text, 200),
                                y, TEXT_SKIP_DRAW, NULL);
}

static void HighlightMenuItem(u8 menuItem)
{
    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(16, DISPLAY_WIDTH - 16));
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(menuItem * 16 + 40, menuItem * 16 + 56));
}

#define TILE_TOP_CORNER_L 0x1A2
#define TILE_TOP_EDGE     0x1A3
#define TILE_TOP_CORNER_R 0x1A4
#define TILE_LEFT_EDGE    0x1A5
#define TILE_RIGHT_EDGE   0x1A7
#define TILE_BOT_CORNER_L 0x1A8
#define TILE_BOT_EDGE     0x1A9
#define TILE_BOT_CORNER_R 0x1AA

static void DrawBgWindowFrames(void)
{
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  0,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  0, 27,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  0,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  1,  1,  2,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  1,  1,  2,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1,  3,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2,  3, 27,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28,  3,  1,  1,  7);

    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  4,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  4, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  4,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  5,  1, 10,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  5,  1, 10,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1, 15,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2, 15, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28, 15,  1,  1,  7);
    CopyBgTilemapBufferToVram(1);
}
