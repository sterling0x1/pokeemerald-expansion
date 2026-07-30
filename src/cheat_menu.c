#include "global.h"
#include "cheat_menu.h"
#include "bg.h"
#include "cheats.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
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

#define tSelection data[0]
#define tScroll data[1]
#define VISIBLE_ROWS 4

enum { WIN_HEADER, WIN_OPTIONS, WIN_DESCRIPTION };

static void MainCB2(void);
static void VBlankCB(void);
static void Task_FadeIn(u8 taskId);
static void Task_ProcessInput(u8 taskId);
static void Task_FadeOut(u8 taskId);
static void DrawMenu(u8 taskId);
static void DrawDescription(u8 taskId);
static void HighlightSelection(u8 taskId);
static void DrawBgWindowFrames(void);

static const u8 sHeader[] = _("CHEAT MENU");
static const u8 sNames[CHEAT_COUNT + 1][24] =
{
    [CHEAT_MONEY_MULTIPLIER] = _("MONEY MULTIPLIER"),
    [CHEAT_EXP_MULTIPLIER]   = _("EXP MULTIPLIER"),
    [CHEAT_CATCH_RATE]       = _("CATCH RATE"),
    [CHEAT_HATCH_SPEED]      = _("EGG HATCH SPEED"),
    [CHEAT_EV_GAIN]          = _("EV GAIN"),
    [CHEAT_MART_PRICES]      = _("POKé MART PRICES"),
    [CHEAT_INFINITE_REPEL]   = _("INFINITE REPEL"),
    [CHEAT_COUNT]            = _("RESET TO DEFAULTS"),
};

static const u8 sOff[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}OFF");
static const u8 sOn[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ON");
static const u8 s1x[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}1×");
static const u8 s2x[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}2×");
static const u8 s3x[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}3×");
static const u8 s4x[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}4×");
static const u8 sNormal[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}NORMAL");
static const u8 sGuaranteed[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}GUARANTEED");
static const u8 sInstant[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}INSTANT");
static const u8 sHalf[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}HALF");
static const u8 sFree[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}FREE");
static const u8 sReset[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}PRESS A");

static const u8 sDescriptions[CHEAT_COUNT + 1][80] =
{
    [CHEAT_MONEY_MULTIPLIER] = _("Multiplies prize money earned after\ntrainer battles."),
    [CHEAT_EXP_MULTIPLIER]   = _("Multiplies all EXP gained by the\nselected amount."),
    [CHEAT_CATCH_RATE]       = _("Raises final capture odds, or makes\nvalid throws guarantee a catch."),
    [CHEAT_HATCH_SPEED]      = _("Reduces Egg cycles while walking.\nINSTANT hatches on the next check."),
    [CHEAT_EV_GAIN]          = _("Disables or multiplies EVs awarded\nafter defeating Pokémon."),
    [CHEAT_MART_PRICES]      = _("Changes displayed and charged prices\nin standard Poké Marts."),
    [CHEAT_INFINITE_REPEL]   = _("Repel steps stop decreasing after a\nRepel has been activated."),
    [CHEAT_COUNT]            = _("Restore every cheat modifier to its\ndefault value. History remains marked."),
};

static const struct WindowTemplate sWindows[] =
{
    [WIN_HEADER] =
    {
        .bg = 1, .tilemapLeft = 2, .tilemapTop = 1, .width = 26, .height = 2,
        .paletteNum = 1, .baseBlock = 2,
    },
    [WIN_OPTIONS] =
    {
        .bg = 0, .tilemapLeft = 2, .tilemapTop = 5, .width = 26, .height = 8,
        .paletteNum = 1, .baseBlock = 0x36,
    },
    [WIN_DESCRIPTION] =
    {
        .bg = 0, .tilemapLeft = 2, .tilemapTop = 15, .width = 26, .height = 4,
        .paletteNum = 1, .baseBlock = 0x106,
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sBgs[] =
{
    {.bg = 1, .charBaseIndex = 1, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0},
    {.bg = 0, .charBaseIndex = 1, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 1, .baseTile = 0},
};

static const u16 sBgPalette[] = {RGB(17, 18, 31)};
static const u16 sTextPalette[] = INCGFX_U16("graphics/interface/option_menu_text.pal", ".gbapal");

static const u8 *GetValueText(u8 option)
{
    u8 value;
    static const u8 *const multipliers[] = {s1x, s2x, s3x, s4x};

    if (option == CHEAT_COUNT)
        return sReset;
    value = Cheats_Get(option);
    switch (option)
    {
    case CHEAT_MONEY_MULTIPLIER:
    case CHEAT_EXP_MULTIPLIER:
        return multipliers[value];
    case CHEAT_CATCH_RATE:
        return value == CHEAT_CATCH_NORMAL ? sNormal : value == CHEAT_CATCH_2X ? s2x : value == CHEAT_CATCH_4X ? s4x : sGuaranteed;
    case CHEAT_HATCH_SPEED:
        return value == CHEAT_HATCH_NORMAL ? sNormal : value == CHEAT_HATCH_2X ? s2x : value == CHEAT_HATCH_4X ? s4x : sInstant;
    case CHEAT_EV_GAIN:
        return value == CHEAT_EV_OFF ? sOff : value == CHEAT_EV_1X ? s1x : value == CHEAT_EV_2X ? s2x : s4x;
    case CHEAT_MART_PRICES:
        return value == CHEAT_MART_NORMAL ? sNormal : value == CHEAT_MART_HALF ? sHalf : sFree;
    case CHEAT_INFINITE_REPEL:
        return value ? sOn : sOff;
    default:
        return sOff;
    }
}

void CB2_InitCheatMenu(void)
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
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_CLR);
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_DARKEN);
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
        LoadBgTiles(1, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1A2);
        LoadPalette(sBgPalette, BG_PLTT_ID(0), sizeof(sBgPalette));
        LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        LoadPalette(sTextPalette, BG_PLTT_ID(1), sizeof(sTextPalette));
        gMain.state++;
        break;
    case 3:
    {
        u8 taskId;
        PutWindowTilemap(WIN_HEADER);
        PutWindowTilemap(WIN_OPTIONS);
        PutWindowTilemap(WIN_DESCRIPTION);
        DrawBgWindowFrames();
        AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, sHeader, 8, 1, TEXT_SKIP_DRAW, NULL);
        CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
        taskId = CreateTask(Task_FadeIn, 0);
        DrawMenu(taskId);
        DrawDescription(taskId);
        HighlightSelection(taskId);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB);
        SetMainCallback2(MainCB2);
        break;
    }
    }
}

static void MainCB2(void) { RunTasks(); AnimateSprites(); BuildOamBuffer(); UpdatePaletteFade(); }
static void VBlankCB(void) { LoadOam(); ProcessSpriteCopyRequests(); TransferPlttBuffer(); }
static void Task_FadeIn(u8 taskId) { if (!gPaletteFade.active) gTasks[taskId].func = Task_ProcessInput; }

static void DrawMenu(u8 taskId)
{
    u8 row;
    FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
    for (row = 0; row < VISIBLE_ROWS; row++)
    {
        u8 option = gTasks[taskId].tScroll + row;
        const u8 *value = GetValueText(option);
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, sNames[option], 8, row * 16 + 1, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, value, GetStringRightAlignXOffset(FONT_NORMAL, value, 200), row * 16 + 1, TEXT_SKIP_DRAW, NULL);
    }
    CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
}

static void DrawDescription(u8 taskId)
{
    FillWindowPixelBuffer(WIN_DESCRIPTION, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_DESCRIPTION, FONT_SMALL, sDescriptions[gTasks[taskId].tSelection], 8, 3, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(WIN_DESCRIPTION, COPYWIN_FULL);
}

static void HighlightSelection(u8 taskId)
{
    u8 row = gTasks[taskId].tSelection - gTasks[taskId].tScroll;
    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(16, DISPLAY_WIDTH - 16));
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(row * 16 + 40, row * 16 + 56));
}

static void ChangeValue(u8 taskId, s8 direction)
{
    u8 option = gTasks[taskId].tSelection;
    u8 value = Cheats_Get(option);
    u8 max = Cheats_GetMax(option);
    value = direction > 0 ? (value == max ? 0 : value + 1) : (value == 0 ? max : value - 1);
    Cheats_Set(option, value);
    DrawMenu(taskId);
    DrawDescription(taskId);
}

static void Task_ProcessInput(u8 taskId)
{
    s16 *selection = &gTasks[taskId].tSelection;
    s16 *scroll = &gTasks[taskId].tScroll;
    bool32 redraw = FALSE;

    if (JOY_NEW(DPAD_UP) && *selection > 0) { (*selection)--; if (*selection < *scroll) (*scroll)--; redraw = TRUE; }
    else if (JOY_NEW(DPAD_DOWN) && *selection < CHEAT_COUNT) { (*selection)++; if (*selection >= *scroll + VISIBLE_ROWS) (*scroll)++; redraw = TRUE; }
    else if (JOY_NEW(DPAD_LEFT) && *selection < CHEAT_COUNT) { ChangeValue(taskId, -1); PlaySE(SE_SELECT); }
    else if ((JOY_NEW(DPAD_RIGHT) || JOY_NEW(A_BUTTON)) && *selection < CHEAT_COUNT) { ChangeValue(taskId, 1); PlaySE(SE_SELECT); }
    else if (JOY_NEW(A_BUTTON) && *selection == CHEAT_COUNT) { Cheats_ResetDefaults(); DrawMenu(taskId); DrawDescription(taskId); PlaySE(SE_SELECT); }
    else if (JOY_NEW(B_BUTTON)) { PlaySE(SE_SELECT); BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK); gTasks[taskId].func = Task_FadeOut; }

    if (redraw) { PlaySE(SE_SELECT); DrawMenu(taskId); DrawDescription(taskId); HighlightSelection(taskId); }
}

static void Task_FadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        SetMainCallback2(gMain.savedCallback);
        FreeAllWindowBuffers();
        DestroyTask(taskId);
    }
}

#define TL 0x1A2
#define TE 0x1A3
#define TR 0x1A4
#define LE 0x1A5
#define RE 0x1A7
#define BL 0x1A8
#define BE 0x1A9
#define BR 0x1AA
static void Frame(u8 top, u8 height)
{
    FillBgTilemapBufferRect(1, TL, 1, top, 1, 1, 7); FillBgTilemapBufferRect(1, TE, 2, top, 26, 1, 7); FillBgTilemapBufferRect(1, TR, 28, top, 1, 1, 7);
    FillBgTilemapBufferRect(1, LE, 1, top + 1, 1, height, 7); FillBgTilemapBufferRect(1, RE, 28, top + 1, 1, height, 7);
    FillBgTilemapBufferRect(1, BL, 1, top + height + 1, 1, 1, 7); FillBgTilemapBufferRect(1, BE, 2, top + height + 1, 26, 1, 7); FillBgTilemapBufferRect(1, BR, 28, top + height + 1, 1, 1, 7);
}
static void DrawBgWindowFrames(void) { Frame(0, 2); Frame(4, 8); Frame(14, 4); CopyBgTilemapBufferToVram(1); }
