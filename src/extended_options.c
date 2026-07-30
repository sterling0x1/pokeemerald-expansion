#include "global.h"
#include "event_data.h"
#include "extended_options.h"
#include "nuzlocke.h"
#include "constants/difficulty.h"

#define EXTENDED_OPTIONS_MAGIC 0x2E36

struct OptionBits
{
    u8 word;
    u8 shift;
    u8 mask;
};

static const struct OptionBits sOptionBits[EXT_OPT_COUNT] =
{
    [EXT_OPT_AUTO_RUN]             = {0,  0, 0x1},
    [EXT_OPT_RUNNING_INDOORS]      = {0,  1, 0x1},
    [EXT_OPT_ITEM_DESCRIPTIONS]    = {0,  2, 0x3},
    [EXT_OPT_REPEL_PROMPT]         = {0,  4, 0x1},
    [EXT_OPT_BATTLE_SPEED]         = {0,  5, 0x3},
    [EXT_OPT_FAST_BATTLE_INTRO]    = {0,  7, 0x1},
    [EXT_OPT_FAST_HP_BARS]         = {0,  8, 0x1},
    [EXT_OPT_FAST_EXP_BARS]        = {0,  9, 0x1},
    [EXT_OPT_MOVE_INFO]            = {0, 10, 0x1},
    [EXT_OPT_EFFECTIVENESS_HINTS]  = {0, 11, 0x1},
    [EXT_OPT_OPPONENT_INFO]        = {0, 12, 0x1},
    [EXT_OPT_BENCH_ATTACKER]       = {0, 13, 0x1},
    [EXT_OPT_REUSABLE_TMS]         = {0, 14, 0x1},
    [EXT_OPT_EXP_ON_CATCH]         = {0, 15, 0x1},
    [EXT_OPT_PARTY_EXP_SHARE]      = {1,  0, 0x1},
    [EXT_OPT_FIELD_POISON]         = {1,  1, 0x1},
    [EXT_OPT_TRAINER_ESCAPE]       = {1,  2, 0x1},
    [EXT_OPT_DIFFICULTY]           = {1,  3, 0x3},
    [EXT_OPT_ENCOUNTER_STYLE]      = {1,  5, 0x1},
    [EXT_OPT_FOLLOWER]             = {1,  6, 0x1},
    [EXT_OPT_SHINY_ODDS]           = {2,  0, 0xF},
    [EXT_OPT_EXP_MULTIPLIER]       = {2,  4, 0x3},
    [EXT_OPT_NUZLOCKE_PRESET]      = {2,  6, 0x3},
    [EXT_OPT_NUZLOCKE_PERMADEATH]  = {1,  7, 0x1},
    [EXT_OPT_NUZLOCKE_ENCOUNTERS]  = {1,  8, 0x1},
    [EXT_OPT_NUZLOCKE_WHITEOUT]    = {1,  9, 0x1},
    [EXT_OPT_NUZLOCKE_DUPES]       = {2,  8, 0x3},
    [EXT_OPT_NUZLOCKE_SHINY_CLAUSE]= {2, 10, 0x3},
    [EXT_OPT_NUZLOCKE_GIFTS]       = {2, 12, 0x3},
    [EXT_OPT_NUZLOCKE_BATTLE_ITEMS]= {2, 14, 0x3},
    [EXT_OPT_LEVEL_CAPS]           = {1, 10, 0x3},
    [EXT_OPT_NUZLOCKE]             = {1, 12, 0x1},
    [EXT_OPT_RANDOM_TRAINERS]      = {1, 13, 0x1},
    [EXT_OPT_RANDOM_GIFTS_STATIC]  = {1, 14, 0x1},
    [EXT_OPT_RANDOM_ALL_DATA]      = {1, 15, 0x1},
};

static u16 GetWord(u8 word)
{
    switch (word)
    {
    case 0:
        return VarGet(VAR_EXTENDED_OPTIONS_1);
    case 1:
        return VarGet(VAR_EXTENDED_OPTIONS_2);
    default:
        return VarGet(VAR_EXTENDED_OPTIONS_3);
    }
}

static void SetWord(u8 word, u16 value)
{
    switch (word)
    {
    case 0:
        VarSet(VAR_EXTENDED_OPTIONS_1, value);
        break;
    case 1:
        VarSet(VAR_EXTENDED_OPTIONS_2, value);
        break;
    default:
        VarSet(VAR_EXTENDED_OPTIONS_3, value);
        break;
    }
}

u8 ExtendedOptions_GetMax(enum ExtendedOption option)
{
    switch (option)
    {
    case EXT_OPT_ITEM_DESCRIPTIONS:
    case EXT_OPT_BATTLE_SPEED:
    case EXT_OPT_LEVEL_CAPS:
        return 2;
    case EXT_OPT_DIFFICULTY:
        return 2;
    case EXT_OPT_EXP_MULTIPLIER:
        return EXP_MULTIPLIER_4X;
    case EXT_OPT_NUZLOCKE_PRESET:
        return NUZLOCKE_PRESET_CUSTOM;
    case EXT_OPT_NUZLOCKE_DUPES:
        return NUZLOCKE_DUPES_OFF;
    case EXT_OPT_NUZLOCKE_SHINY_CLAUSE:
        return NUZLOCKE_SHINY_OFF;
    case EXT_OPT_NUZLOCKE_GIFTS:
        return NUZLOCKE_GIFTS_FREE;
    case EXT_OPT_NUZLOCKE_BATTLE_ITEMS:
        return NUZLOCKE_BATTLE_ITEMS_BANNED;
    case EXT_OPT_SHINY_ODDS:
        return 8;
    default:
        return 1;
    }
}

u8 ExtendedOptions_Get(enum ExtendedOption option)
{
    const struct OptionBits *bits;

    if (option >= EXT_OPT_COUNT)
        return 0;

    bits = &sOptionBits[option];
    return (GetWord(bits->word) >> bits->shift) & bits->mask;
}

void ExtendedOptions_Set(enum ExtendedOption option, u8 value)
{
    const struct OptionBits *bits;
    u16 word;

    if (option >= EXT_OPT_COUNT)
        return;

    if (Nuzlocke_IsActive() && Nuzlocke_AreSettingsLocked())
    {
        switch (option)
        {
        case EXT_OPT_NUZLOCKE_PRESET:
        case EXT_OPT_NUZLOCKE_PERMADEATH:
        case EXT_OPT_NUZLOCKE_ENCOUNTERS:
        case EXT_OPT_NUZLOCKE_DUPES:
        case EXT_OPT_NUZLOCKE_SHINY_CLAUSE:
        case EXT_OPT_NUZLOCKE_GIFTS:
        case EXT_OPT_NUZLOCKE_WHITEOUT:
        case EXT_OPT_NUZLOCKE_BATTLE_ITEMS:
        case EXT_OPT_LEVEL_CAPS:
            return;
        default:
            break;
        }
    }

    if (value > ExtendedOptions_GetMax(option))
        value = ExtendedOptions_GetMax(option);

    bits = &sOptionBits[option];
    word = GetWord(bits->word);
    word &= ~(bits->mask << bits->shift);
    word |= (value & bits->mask) << bits->shift;
    SetWord(bits->word, word);
}

static void SetExtendedOptionsDefaults(void)
{
    VarSet(VAR_EXTENDED_OPTIONS_1, 0);
    VarSet(VAR_EXTENDED_OPTIONS_2, 0);
    VarSet(VAR_EXTENDED_OPTIONS_3, 0);

    ExtendedOptions_Set(EXT_OPT_RUNNING_INDOORS, TRUE);
    ExtendedOptions_Set(EXT_OPT_REPEL_PROMPT, TRUE);
    ExtendedOptions_Set(EXT_OPT_MOVE_INFO, TRUE);
    ExtendedOptions_Set(EXT_OPT_EFFECTIVENESS_HINTS, TRUE);
    ExtendedOptions_Set(EXT_OPT_OPPONENT_INFO, TRUE);
    ExtendedOptions_Set(EXT_OPT_BENCH_ATTACKER, TRUE);
    ExtendedOptions_Set(EXT_OPT_EXP_ON_CATCH, TRUE);
    ExtendedOptions_Set(EXT_OPT_FIELD_POISON, TRUE);
    ExtendedOptions_Set(EXT_OPT_ENCOUNTER_STYLE, ENCOUNTER_STYLE_VISIBLE);
    ExtendedOptions_Set(EXT_OPT_SHINY_ODDS, SHINY_ODDS_8192);
    ExtendedOptions_Set(EXT_OPT_DIFFICULTY, DIFFICULTY_NORMAL);
}

void ExtendedOptions_InitNewSave(void)
{
    SetExtendedOptionsDefaults();
    VarSet(VAR_EXTENDED_OPTIONS_INITIALIZED, EXTENDED_OPTIONS_MAGIC);
}

void ExtendedOptions_MigrateSave(void)
{
    if (VarGet(VAR_EXTENDED_OPTIONS_INITIALIZED) == EXTENDED_OPTIONS_MAGIC)
        return;

    SetExtendedOptionsDefaults();
    VarSet(VAR_EXTENDED_OPTIONS_INITIALIZED, EXTENDED_OPTIONS_MAGIC);
}
