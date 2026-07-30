#include "global.h"
#include "nuzlocke_menu.h"
#include "bg.h"
#include "extended_options.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "menu.h"
#include "nuzlocke.h"
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
#include "constants/pokemon.h"
#include "constants/songs.h"

#if MODULE_NUZLOCKE_ENABLED
#define tMenuSelection data[0]
#define tPage data[1]

enum
{
    MENUITEM_PRESET,
    MENUITEM_CUSTOM_RULES,
    MENUITEM_LEVEL_CAPS,
    MENUITEM_CONTINUE,
    MENUITEM_COUNT,
};

enum
{
    WIN_HEADER,
    WIN_OPTIONS,
    WIN_DESCRIPTION,
};

enum
{
    PAGE_MAIN,
    PAGE_CUSTOM_1,
    PAGE_CUSTOM_2,
    PAGE_CUSTOM_3,
};

static void CB2_InitNuzlockeSetupMenu(void);
static void MainCB2(void);
static void VBlankCB(void);
static void Task_FadeIn(u8 taskId);
static void Task_ProcessInput(u8 taskId);
static void Task_FadeOut(u8 taskId);
static void DrawHeader(u8 taskId);
static void DrawMenu(u8 taskId);
static void DrawValue(u8 taskId, u8 menuItem);
static void ChangeValue(u8 taskId, u8 menuItem, s8 direction);
static void DrawDescription(u8 taskId);
static void OpenPage(u8 taskId, u8 page);
static enum ExtendedOption GetCustomOption(u8 page, u8 menuItem);
static const u8 *GetCustomName(u8 page, u8 menuItem);
static const u8 *GetCustomValue(enum ExtendedOption option);
static void HighlightMenuItem(u8 menuItem);
static void DrawBgWindowFrames(void);
static void ChangeMonotype(s8 direction);

static MainCallback sBackCallback;

static const u8 sText_Header[] = _("NUZLOCKE SETUP");
static const u8 sText_Preset[] = _("PRESET");
static const u8 sText_CustomRules[] = _("CUSTOM RULES");
static const u8 sText_Type[] = _("TYPE");
static const u8 sText_LevelCaps[] = _("LEVEL CAPS");
static const u8 sText_Continue[] = _("CONTINUE");
static const u8 sText_Next[] = _("NEXT PAGE");
static const u8 sText_Done[] = _("DONE");
static const u8 sText_Permadeath[] = _("PERMADEATH");
static const u8 sText_Encounters[] = _("ENCOUNTERS");
static const u8 sText_Dupes[] = _("DUPES CLAUSE");
static const u8 sText_Shiny[] = _("SHINY CLAUSE");
static const u8 sText_Gifts[] = _("GIFTS/STATIC");
static const u8 sText_Whiteout[] = _("WHITEOUT");
static const u8 sText_BattleItems[] = _("BATTLE ITEMS");
static const u8 sText_BattleStyle[] = _("BATTLE STYLE");
static const u8 sText_Standard[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}STANDARD");
static const u8 sText_Hardcore[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}HARDCORE");
static const u8 sText_Monotype[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}MONOTYPE");
static const u8 sText_Custom[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}CUSTOM");
static const u8 sText_Off[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}OFF");
static const u8 sText_Soft[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}SOFT");
static const u8 sText_Hard[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}HARD");
static const u8 sText_Box[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}BOX");
static const u8 sText_Release[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}RELEASE");
static const u8 sText_FirstArea[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}FIRST/AREA");
static const u8 sText_Family[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}FAMILY");
static const u8 sText_Species[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}SPECIES");
static const u8 sText_Replace[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}REPLACE");
static const u8 sText_Collection[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}COLLECT");
static const u8 sText_Free[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}FREE");
static const u8 sText_Separate[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}SEPARATE");
static const u8 sText_Count[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}COUNT");
static const u8 sText_GameOver[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}GAME OVER");
static const u8 sText_Allowed[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ALLOWED");
static const u8 sText_One[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ONE");
static const u8 sText_Banned[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}BANNED");
static const u8 sText_PlayerChoice[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}CHOICE");
static const u8 sText_Set[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}SET");
static const u8 sText_Open[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}OPEN");
static const u8 sText_Locked[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}LOCKED");

static const u8 sText_TypeNormal[]   = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}NORMAL");
static const u8 sText_TypeFighting[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}FIGHTING");
static const u8 sText_TypeFlying[]   = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}FLYING");
static const u8 sText_TypePoison[]   = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}POISON");
static const u8 sText_TypeGround[]   = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}GROUND");
static const u8 sText_TypeRock[]     = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ROCK");
static const u8 sText_TypeBug[]      = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}BUG");
static const u8 sText_TypeGhost[]    = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}GHOST");
static const u8 sText_TypeSteel[]    = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}STEEL");
static const u8 sText_TypeFire[]     = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}FIRE");
static const u8 sText_TypeWater[]    = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}WATER");
static const u8 sText_TypeGrass[]    = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}GRASS");
static const u8 sText_TypeElectric[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ELECTRIC");
static const u8 sText_TypePsychic[]  = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}PSYCHIC");
static const u8 sText_TypeIce[]      = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ICE");
static const u8 sText_TypeDragon[]   = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}DRAGON");
static const u8 sText_TypeDark[]     = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}DARK");
static const u8 sText_TypeFairy[]    = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}FAIRY");

static const u8 sMonotypeTypes[] =
{
    TYPE_NORMAL, TYPE_FIGHTING, TYPE_FLYING, TYPE_POISON, TYPE_GROUND, TYPE_ROCK,
    TYPE_BUG, TYPE_GHOST, TYPE_STEEL, TYPE_FIRE, TYPE_WATER, TYPE_GRASS,
    TYPE_ELECTRIC, TYPE_PSYCHIC, TYPE_ICE, TYPE_DRAGON, TYPE_DARK, TYPE_FAIRY,
};

static const u8 *const sMonotypeNames[] =
{
    sText_TypeNormal, sText_TypeFighting, sText_TypeFlying, sText_TypePoison,
    sText_TypeGround, sText_TypeRock, sText_TypeBug, sText_TypeGhost,
    sText_TypeSteel, sText_TypeFire, sText_TypeWater, sText_TypeGrass,
    sText_TypeElectric, sText_TypePsychic, sText_TypeIce, sText_TypeDragon,
    sText_TypeDark, sText_TypeFairy,
};

static const u8 sDesc_Standard[] = _("Classic first-catch and permadeath.\nItems allowed; level caps optional.");
static const u8 sDesc_Hardcore[] = _("Hard caps, SET mode, and no Bag items.\nA full whiteout ends the run.");
static const u8 sDesc_Monotype[] = _("All usable Pokémon share one type.\nType selection follows this menu.");
static const u8 sDesc_Custom[] = _("Build your own rules for encounters,\nfainting, clauses, caps, and items.");
static const u8 sDesc_CustomRulesOpen[] = _("Press A to edit all Custom rules.\nChoices are saved with this run.");
static const u8 sDesc_CustomRulesLocked[] = _("Select the CUSTOM preset to unlock\nand edit the individual rule pages.");
static const u8 sDesc_Type[] = _("Choose the only Pokémon type permitted\nfor this Monotype Nuzlocke run.");
static const u8 sDesc_LevelCaps[] = _("SOFT reduces EXP at badge caps. HARD\nstops EXP and Candy at the cap.");
static const u8 sDesc_Continue[] = _("Accept these rules and continue to\nthe Randomizer Setup.");
static const u8 sDesc_Permadeath[] = _("Fainted Pokémon cannot be used again.\nSend them to a death box or release.");
static const u8 sDesc_Encounters[] = _("Allow only the first valid encounter\nin each named area, or disable limits.");
static const u8 sDesc_Dupes[] = _("Skip duplicates by species or family,\nor allow duplicate encounters.");
static const u8 sDesc_Shiny[] = _("Shinies may consume, replace, or ignore\nan area's encounter limit.");
static const u8 sDesc_Gifts[] = _("Gifts/statics may have a separate limit,\ncount for the area, or remain free.");
static const u8 sDesc_Whiteout[] = _("Continue using boxed Pokémon, or end\nthe run when the full party falls.");
static const u8 sDesc_BattleItems[] = _("Allow all, one, or no Bag items in\ntrainer battles. Held items stay legal.");
static const u8 sDesc_BattleStyle[] = _("CHOICE keeps your setting. SET removes\nthe free switch after a foe faints.");
static const u8 sDesc_Next[] = _("Continue to the next Custom Rules page.\nChoices on this page are kept.");
static const u8 sDesc_Done[] = _("Keep all Custom Rule choices and go\nback to the main Nuzlocke Setup page.");

static const u8 *const sMenuItemNames[MENUITEM_COUNT] =
{
    [MENUITEM_PRESET] = sText_Preset,
    [MENUITEM_CUSTOM_RULES] = sText_CustomRules,
    [MENUITEM_LEVEL_CAPS] = sText_LevelCaps,
    [MENUITEM_CONTINUE] = sText_Continue,
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
        .height = 8,
        .paletteNum = 1,
        .baseBlock = 0x36,
    },
    [WIN_DESCRIPTION] = {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 26,
        .height = 4,
        .paletteNum = 1,
        .baseBlock = 0x106,
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

void Nuzlocke_ApplyPreset(u8 preset)
{
    ExtendedOptions_Set(EXT_OPT_NUZLOCKE_PRESET, preset);

    if (preset == NUZLOCKE_PRESET_CUSTOM)
        return;

    if (preset == NUZLOCKE_PRESET_MONOTYPE && Nuzlocke_GetMonotype() == NUZLOCKE_NO_TYPE)
        Nuzlocke_SetMonotype(TYPE_NORMAL);

    ExtendedOptions_Set(EXT_OPT_NUZLOCKE_PERMADEATH, NUZLOCKE_PERMADEATH_BOX);
    ExtendedOptions_Set(EXT_OPT_NUZLOCKE_ENCOUNTERS, NUZLOCKE_ENCOUNTERS_FIRST_AREA);
    ExtendedOptions_Set(EXT_OPT_NUZLOCKE_DUPES, NUZLOCKE_DUPES_FAMILY);
    ExtendedOptions_Set(EXT_OPT_NUZLOCKE_SHINY_CLAUSE, NUZLOCKE_SHINY_REPLACE);
    ExtendedOptions_Set(EXT_OPT_NUZLOCKE_GIFTS, NUZLOCKE_GIFTS_SEPARATE);

    if (preset == NUZLOCKE_PRESET_HARDCORE)
    {
        ExtendedOptions_Set(EXT_OPT_LEVEL_CAPS, LEVEL_CAPS_HARD);
        ExtendedOptions_Set(EXT_OPT_NUZLOCKE_WHITEOUT, NUZLOCKE_WHITEOUT_GAME_OVER);
        ExtendedOptions_Set(EXT_OPT_NUZLOCKE_BATTLE_ITEMS, NUZLOCKE_BATTLE_ITEMS_BANNED);
        gSaveBlock2Ptr->optionsBattleStyle = OPTIONS_BATTLE_STYLE_SET;
    }
    else
    {
        ExtendedOptions_Set(EXT_OPT_LEVEL_CAPS, LEVEL_CAPS_OFF);
        ExtendedOptions_Set(EXT_OPT_NUZLOCKE_WHITEOUT, NUZLOCKE_WHITEOUT_CONTINUE);
        ExtendedOptions_Set(EXT_OPT_NUZLOCKE_BATTLE_ITEMS, NUZLOCKE_BATTLE_ITEMS_ALLOWED);
        gSaveBlock2Ptr->optionsBattleStyle = OPTIONS_BATTLE_STYLE_SHIFT;
    }
}

void StartNuzlockeSetupMenu(MainCallback returnCallback)
{
    StartNuzlockeSetupMenuWithBack(returnCallback, NULL);
}

void StartNuzlockeSetupMenuWithBack(MainCallback returnCallback, MainCallback backCallback)
{
    gMain.savedCallback = returnCallback;
    sBackCallback = backCallback;
    gMain.state = 0;
    SetMainCallback2(CB2_InitNuzlockeSetupMenu);
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

static void CB2_InitNuzlockeSetupMenu(void)
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
        PutWindowTilemap(WIN_DESCRIPTION);
        DrawBgWindowFrames();
        gMain.state++;
        break;
    case 5:
    {
        u8 taskId = CreateTask(Task_FadeIn, 0);

        gTasks[taskId].tMenuSelection = MENUITEM_PRESET;
        gTasks[taskId].tPage = PAGE_MAIN;
        DrawHeader(taskId);
        DrawMenu(taskId);
        DrawDescription(taskId);
        HighlightMenuItem(MENUITEM_PRESET);
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
    u8 page = gTasks[taskId].tPage;
    u8 lastItem = page == PAGE_MAIN ? MENUITEM_CONTINUE : 3;

    if (JOY_NEW(DPAD_UP))
    {
        selection = (selection == 0) ? lastItem : selection - 1;
        gTasks[taskId].tMenuSelection = selection;
        HighlightMenuItem(selection);
        DrawDescription(taskId);
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        selection = (selection == lastItem) ? 0 : selection + 1;
        gTasks[taskId].tMenuSelection = selection;
        HighlightMenuItem(selection);
        DrawDescription(taskId);
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(DPAD_LEFT))
    {
        if (page == PAGE_MAIN && selection == MENUITEM_CUSTOM_RULES
         && ExtendedOptions_Get(EXT_OPT_NUZLOCKE_PRESET) == NUZLOCKE_PRESET_MONOTYPE)
        {
            ChangeMonotype(-1);
            DrawValue(taskId, selection);
            CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
            PlaySE(SE_SELECT);
        }
        else if (page != PAGE_MAIN || selection == MENUITEM_PRESET || selection == MENUITEM_LEVEL_CAPS)
        {
            ChangeValue(taskId, selection, -1);
            PlaySE(SE_SELECT);
        }
    }
    else if (JOY_NEW(DPAD_RIGHT))
    {
        if (page == PAGE_MAIN && selection == MENUITEM_CUSTOM_RULES
         && ExtendedOptions_Get(EXT_OPT_NUZLOCKE_PRESET) == NUZLOCKE_PRESET_MONOTYPE)
        {
            ChangeMonotype(1);
            DrawValue(taskId, selection);
            CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
            PlaySE(SE_SELECT);
        }
        else if (page != PAGE_MAIN || selection == MENUITEM_PRESET || selection == MENUITEM_LEVEL_CAPS)
        {
            ChangeValue(taskId, selection, 1);
            PlaySE(SE_SELECT);
        }
    }
    else if (JOY_NEW(A_BUTTON))
    {
        if (page == PAGE_MAIN && selection == MENUITEM_CONTINUE)
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            gTasks[taskId].func = Task_FadeOut;
        }
        else if (page == PAGE_MAIN
              && selection == MENUITEM_CUSTOM_RULES
              && ExtendedOptions_Get(EXT_OPT_NUZLOCKE_PRESET) == NUZLOCKE_PRESET_CUSTOM)
        {
            OpenPage(taskId, PAGE_CUSTOM_1);
            PlaySE(SE_SELECT);
        }
        else if (page == PAGE_MAIN
              && selection == MENUITEM_CUSTOM_RULES
              && ExtendedOptions_Get(EXT_OPT_NUZLOCKE_PRESET) == NUZLOCKE_PRESET_MONOTYPE)
        {
            ChangeMonotype(1);
            DrawValue(taskId, selection);
            CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
            PlaySE(SE_SELECT);
        }
        else if (page != PAGE_MAIN && selection == 3)
        {
            OpenPage(taskId, page == PAGE_CUSTOM_3 ? PAGE_MAIN : page + 1);
            PlaySE(SE_SELECT);
        }
        else if (page != PAGE_MAIN || selection == MENUITEM_PRESET || selection == MENUITEM_LEVEL_CAPS)
        {
            ChangeValue(taskId, selection, 1);
            PlaySE(SE_SELECT);
        }
        else
        {
            PlaySE(SE_BOO);
        }
    }
    else if (JOY_NEW(B_BUTTON) && page != PAGE_MAIN)
    {
        OpenPage(taskId, PAGE_MAIN);
        PlaySE(SE_SELECT);
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

static void DrawHeader(u8 taskId)
{
    u8 header[24];

    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
    if (gTasks[taskId].tPage == PAGE_MAIN)
    {
        StringCopy(header, sText_Header);
    }
    else
    {
        StringCopy(header, COMPOUND_STRING("CUSTOM RULES "));
        ConvertIntToDecimalStringN(header + StringLength(header), gTasks[taskId].tPage, STR_CONV_MODE_LEFT_ALIGN, 1);
        StringAppend(header, COMPOUND_STRING("/3"));
    }
    AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, header, 8, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static enum ExtendedOption GetCustomOption(u8 page, u8 menuItem)
{
    static const enum ExtendedOption sCustomOptions[3][3] =
    {
        {EXT_OPT_NUZLOCKE_PERMADEATH, EXT_OPT_NUZLOCKE_ENCOUNTERS, EXT_OPT_NUZLOCKE_DUPES},
        {EXT_OPT_NUZLOCKE_SHINY_CLAUSE, EXT_OPT_NUZLOCKE_GIFTS, EXT_OPT_NUZLOCKE_WHITEOUT},
        {EXT_OPT_NUZLOCKE_BATTLE_ITEMS, EXT_OPT_LEVEL_CAPS, EXT_OPT_COUNT},
    };

    return sCustomOptions[page - PAGE_CUSTOM_1][menuItem];
}

static const u8 *GetCustomName(u8 page, u8 menuItem)
{
    static const u8 *const sCustomNames[3][3] =
    {
        {sText_Permadeath, sText_Encounters, sText_Dupes},
        {sText_Shiny, sText_Gifts, sText_Whiteout},
        {sText_BattleItems, sText_LevelCaps, sText_BattleStyle},
    };

    return sCustomNames[page - PAGE_CUSTOM_1][menuItem];
}

static const u8 *GetCustomValue(enum ExtendedOption option)
{
    u8 value = ExtendedOptions_Get(option);

    switch (option)
    {
    case EXT_OPT_NUZLOCKE_PERMADEATH:
        return value == NUZLOCKE_PERMADEATH_BOX ? sText_Box : sText_Release;
    case EXT_OPT_NUZLOCKE_ENCOUNTERS:
        return value == NUZLOCKE_ENCOUNTERS_FIRST_AREA ? sText_FirstArea : sText_Off;
    case EXT_OPT_NUZLOCKE_DUPES:
        return value == NUZLOCKE_DUPES_FAMILY ? sText_Family : value == NUZLOCKE_DUPES_SPECIES ? sText_Species : sText_Off;
    case EXT_OPT_NUZLOCKE_SHINY_CLAUSE:
        return value == NUZLOCKE_SHINY_REPLACE ? sText_Replace
             : value == NUZLOCKE_SHINY_COLLECTION ? sText_Collection
             : value == NUZLOCKE_SHINY_FREE ? sText_Free
             : sText_Off;
    case EXT_OPT_NUZLOCKE_GIFTS:
        return value == NUZLOCKE_GIFTS_SEPARATE ? sText_Separate : value == NUZLOCKE_GIFTS_COUNT ? sText_Count : sText_Free;
    case EXT_OPT_NUZLOCKE_WHITEOUT:
        return value == NUZLOCKE_WHITEOUT_CONTINUE ? sText_Continue : sText_GameOver;
    case EXT_OPT_NUZLOCKE_BATTLE_ITEMS:
        return value == NUZLOCKE_BATTLE_ITEMS_ALLOWED ? sText_Allowed : value == NUZLOCKE_BATTLE_ITEMS_ONE ? sText_One : sText_Banned;
    case EXT_OPT_LEVEL_CAPS:
        return value == LEVEL_CAPS_OFF ? sText_Off : value == LEVEL_CAPS_NORMAL ? sText_Soft : sText_Hard;
    default:
        return sText_Off;
    }
}

static void DrawMenu(u8 taskId)
{
    u8 i;
    u8 page = gTasks[taskId].tPage;

    FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
    if (page == PAGE_MAIN)
    {
        for (i = 0; i < MENUITEM_COUNT; i++)
        {
            const u8 *name = (i == MENUITEM_CUSTOM_RULES
                           && ExtendedOptions_Get(EXT_OPT_NUZLOCKE_PRESET) == NUZLOCKE_PRESET_MONOTYPE)
                           ? sText_Type
                           : sMenuItemNames[i];
            AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, name, 8, i * 16 + 1, TEXT_SKIP_DRAW, NULL);
        }
        DrawValue(taskId, MENUITEM_PRESET);
        DrawValue(taskId, MENUITEM_CUSTOM_RULES);
        DrawValue(taskId, MENUITEM_LEVEL_CAPS);
    }
    else
    {
        for (i = 0; i < 3; i++)
        {
            AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, GetCustomName(page, i), 8, i * 16 + 1, TEXT_SKIP_DRAW, NULL);
            DrawValue(taskId, i);
        }
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, page == PAGE_CUSTOM_3 ? sText_Done : sText_Next, 8, 3 * 16 + 1, TEXT_SKIP_DRAW, NULL);
    }
    CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
}

static void DrawValue(u8 taskId, u8 menuItem)
{
    const u8 *text;
    u8 y = menuItem * 16 + 1;
    u8 page = gTasks[taskId].tPage;

    if (page != PAGE_MAIN)
    {
        enum ExtendedOption option = GetCustomOption(page, menuItem);
        text = option == EXT_OPT_COUNT
             ? (gSaveBlock2Ptr->optionsBattleStyle == OPTIONS_BATTLE_STYLE_SET ? sText_Set : sText_PlayerChoice)
             : GetCustomValue(option);
    }
    else if (menuItem == MENUITEM_PRESET)
    {
        switch (ExtendedOptions_Get(EXT_OPT_NUZLOCKE_PRESET))
        {
        case NUZLOCKE_PRESET_HARDCORE: text = sText_Hardcore; break;
        case NUZLOCKE_PRESET_MONOTYPE: text = sText_Monotype; break;
        case NUZLOCKE_PRESET_CUSTOM: text = sText_Custom; break;
        case NUZLOCKE_PRESET_STANDARD:
        default: text = sText_Standard; break;
        }
    }
    else if (menuItem == MENUITEM_CUSTOM_RULES)
    {
        if (ExtendedOptions_Get(EXT_OPT_NUZLOCKE_PRESET) == NUZLOCKE_PRESET_MONOTYPE)
        {
            u8 i;
            text = sMonotypeNames[0];
            for (i = 0; i < ARRAY_COUNT(sMonotypeTypes); i++)
            {
                if (sMonotypeTypes[i] == Nuzlocke_GetMonotype())
                {
                    text = sMonotypeNames[i];
                    break;
                }
            }
        }
        else
        {
            text = ExtendedOptions_Get(EXT_OPT_NUZLOCKE_PRESET) == NUZLOCKE_PRESET_CUSTOM ? sText_Open : sText_Locked;
        }
    }
    else
    {
        text = GetCustomValue(EXT_OPT_LEVEL_CAPS);
    }

    FillWindowPixelRect(WIN_OPTIONS, PIXEL_FILL(1), 104, y, 96, 14);
    AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, text,
                                GetStringRightAlignXOffset(FONT_NORMAL, text, 200),
                                y, TEXT_SKIP_DRAW, NULL);
}

static void ChangeMonotype(s8 direction)
{
    u8 i;
    u8 currentType = Nuzlocke_GetMonotype();

    for (i = 0; i < ARRAY_COUNT(sMonotypeTypes); i++)
    {
        if (sMonotypeTypes[i] == currentType)
            break;
    }
    if (i == ARRAY_COUNT(sMonotypeTypes))
        i = 0;
    else if (direction > 0)
        i = (i + 1) % ARRAY_COUNT(sMonotypeTypes);
    else
        i = (i == 0) ? ARRAY_COUNT(sMonotypeTypes) - 1 : i - 1;

    Nuzlocke_SetMonotype(sMonotypeTypes[i]);
}

static void ChangeValue(u8 taskId, u8 menuItem, s8 direction)
{
    u8 page = gTasks[taskId].tPage;
    enum ExtendedOption option = page == PAGE_MAIN
                               ? (menuItem == MENUITEM_PRESET ? EXT_OPT_NUZLOCKE_PRESET : EXT_OPT_LEVEL_CAPS)
                               : GetCustomOption(page, menuItem);
    u8 value;
    u8 max;

    if (option == EXT_OPT_COUNT)
    {
        gSaveBlock2Ptr->optionsBattleStyle ^= 1;
        DrawValue(taskId, menuItem);
        DrawDescription(taskId);
        CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
        return;
    }

    value = ExtendedOptions_Get(option);
    max = ExtendedOptions_GetMax(option);

    value = direction > 0 ? (value == max ? 0 : value + 1) : (value == 0 ? max : value - 1);
    ExtendedOptions_Set(option, value);

    if (page == PAGE_MAIN && menuItem == MENUITEM_PRESET)
    {
        Nuzlocke_ApplyPreset(value);
        DrawMenu(taskId);
    }
    else if (page == PAGE_MAIN)
    {
        ExtendedOptions_Set(EXT_OPT_NUZLOCKE_PRESET, NUZLOCKE_PRESET_CUSTOM);
        DrawValue(taskId, MENUITEM_PRESET);
        DrawValue(taskId, MENUITEM_CUSTOM_RULES);
    }

    DrawValue(taskId, menuItem);
    DrawDescription(taskId);
    CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
}

static void DrawDescription(u8 taskId)
{
    const u8 *text;
    u8 page = gTasks[taskId].tPage;
    u8 selection = gTasks[taskId].tMenuSelection;

    if (page == PAGE_MAIN)
    {
        if (selection == MENUITEM_CUSTOM_RULES)
        {
            if (ExtendedOptions_Get(EXT_OPT_NUZLOCKE_PRESET) == NUZLOCKE_PRESET_MONOTYPE)
                text = sDesc_Type;
            else
                text = ExtendedOptions_Get(EXT_OPT_NUZLOCKE_PRESET) == NUZLOCKE_PRESET_CUSTOM
                     ? sDesc_CustomRulesOpen
                     : sDesc_CustomRulesLocked;
        }
        else if (selection == MENUITEM_LEVEL_CAPS)
            text = sDesc_LevelCaps;
        else if (selection == MENUITEM_CONTINUE)
            text = sDesc_Continue;
        else
        {
            switch (ExtendedOptions_Get(EXT_OPT_NUZLOCKE_PRESET))
            {
            case NUZLOCKE_PRESET_HARDCORE: text = sDesc_Hardcore; break;
            case NUZLOCKE_PRESET_MONOTYPE: text = sDesc_Monotype; break;
            case NUZLOCKE_PRESET_CUSTOM: text = sDesc_Custom; break;
            case NUZLOCKE_PRESET_STANDARD:
            default: text = sDesc_Standard; break;
            }
        }
    }
    else if (selection == 3)
    {
        text = page == PAGE_CUSTOM_3 ? sDesc_Done : sDesc_Next;
    }
    else
    {
        static const u8 *const sDescriptions[3][3] =
        {
            {sDesc_Permadeath, sDesc_Encounters, sDesc_Dupes},
            {sDesc_Shiny, sDesc_Gifts, sDesc_Whiteout},
            {sDesc_BattleItems, sDesc_LevelCaps, sDesc_BattleStyle},
        };
        text = sDescriptions[page - PAGE_CUSTOM_1][selection];
    }

    FillWindowPixelBuffer(WIN_DESCRIPTION, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_DESCRIPTION, FONT_SMALL, text, 8, 3, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(WIN_DESCRIPTION, COPYWIN_FULL);
}

static void OpenPage(u8 taskId, u8 page)
{
    gTasks[taskId].tPage = page;
    gTasks[taskId].tMenuSelection = 0;
    DrawHeader(taskId);
    DrawMenu(taskId);
    DrawDescription(taskId);
    HighlightMenuItem(0);
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
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  5,  1,  8,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  5,  1,  8,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1, 13,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2, 13, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28, 13,  1,  1,  7);

    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1, 14,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2, 14, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28, 14,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1, 15,  1,  4,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28, 15,  1,  4,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1, 19,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2, 19, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28, 19,  1,  1,  7);
    CopyBgTilemapBufferToVram(1);
}

#endif // MODULE_NUZLOCKE_ENABLED
