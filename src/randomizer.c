#include "global.h"
#include "event_data.h"
#include "extended_options.h"
#include "item.h"
#include "module_save.h"
#include "move.h"
#include "nuzlocke.h"
#include "pokemon.h"
#include "random.h"
#include "randomizer.h"
#include "config/randomizer.h"
#include "constants/pokedex.h"
#include "constants/abilities.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/moves.h"

#if MODULE_RANDOMIZER_ENABLED

#define RANDOMIZER_SAVE_MAGIC   0x524E4431
#define RANDOMIZER_SAVE_VERSION 1

struct RandomizerSaveData
{
    u32 magic;
    u32 seed;
    u16 version;
    u8 wildEnabled:1;
    u8 startersEnabled:1;
    u8 trainersEnabled:1;
    u8 giftsStaticEnabled:1;
    u8 allDataEnabled:1;
    u8 movesEnabled:1;
    u8 abilitiesEnabled:1;
    u8 evolutionsEnabled:1;
    u8 itemsEnabled:1;
    u8 unused:7;
};

STATIC_ASSERT(sizeof(struct RandomizerSaveData) == 12, RandomizerSaveDataSize);

static struct RandomizerSaveData *GetSaveData(void)
{
    u16 version;
    u16 size;
    struct RandomizerSaveData *data = ModuleSave_GetChunk(MODULE_ID_RANDOMIZER, &version, &size);

    if (data == NULL || version != RANDOMIZER_SAVE_VERSION || size != sizeof(*data))
        return NULL;
    return data;
}

bool32 Randomizer_IsInitialized(void)
{
    return GetSaveData() != NULL;
}

void Randomizer_LoadSave(void)
{
    struct RandomizerSaveData legacy;
    struct RandomizerSaveData *data;
    u32 legacySeed;

    data = GetSaveData();
    if (data != NULL)
        return;

    memcpy(&legacy, gSaveBlock3Ptr->moduleLegacyData, sizeof(legacy));
    legacySeed = VarGet(VAR_RANDOMIZER_SEED_LOW)
               | ((u32)VarGet(VAR_RANDOMIZER_SEED_HIGH) << 16);
    data = ModuleSave_RecreateChunk(MODULE_ID_RANDOMIZER, RANDOMIZER_SAVE_VERSION, sizeof(*data));
    if (data == NULL)
        return;
    data->magic = RANDOMIZER_SAVE_MAGIC;
    data->version = RANDOMIZER_SAVE_VERSION;

    if (legacy.magic == RANDOMIZER_SAVE_MAGIC && legacy.version == RANDOMIZER_SAVE_VERSION)
    {
        *data = legacy;
    }
    else if (VarGet(VAR_RANDOMIZER_SETTINGS_INITIALIZED) == TRUE)
    {
        data->seed = legacySeed != 0 ? legacySeed : RANDOMIZER_FALLBACK_SEED;
        data->wildEnabled = VarGet(VAR_RANDOMIZER_WILD_ENABLED) != FALSE;
        data->startersEnabled = VarGet(VAR_RANDOMIZER_STARTERS_ENABLED) != FALSE;
        data->trainersEnabled = ExtendedOptions_Get(EXT_OPT_RANDOM_TRAINERS);
        data->giftsStaticEnabled = ExtendedOptions_Get(EXT_OPT_RANDOM_GIFTS_STATIC);
        data->allDataEnabled = ExtendedOptions_Get(EXT_OPT_RANDOM_ALL_DATA);
    }
    else
    {
        // Saves created before the Randomizer existed remain non-randomized.
        data->seed = RANDOMIZER_FALLBACK_SEED;
    }
}

u32 Randomizer_GetSeed(void)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();

    return data != NULL && data->seed != 0
         ? data->seed
         : RANDOMIZER_FALLBACK_SEED;
}

void Randomizer_InitNewGameSeed(void)
{
    struct RandomizerSaveData *data = ModuleSave_RecreateChunk(MODULE_ID_RANDOMIZER, RANDOMIZER_SAVE_VERSION, sizeof(*data));

    if (data == NULL)
        return;
    data->magic = RANDOMIZER_SAVE_MAGIC;
    data->version = RANDOMIZER_SAVE_VERSION;
    Randomizer_RerollSeed();

    Randomizer_SetWildEnabled(RANDOMIZER_WILD_POKEMON);
    Randomizer_SetStarterEnabled(RANDOMIZER_STARTERS);

    // Use the expansion's existing National Dex enable routine here.
    // Replace EnableNationalPokedex() with the correct helper if the project
    // uses a different function name.
    EnableNationalPokedex();

}

void Randomizer_RerollSeed(void)
{
    struct RandomizerSaveData *data;
    u32 oldSeed = Randomizer_GetSeed();
    u32 seed;

    do
    {
        seed = Random32();
    } while (seed == 0 || seed == oldSeed);

    data = GetSaveData();
    if (data != NULL)
        data->seed = seed;
}

bool32 Randomizer_IsWildEnabled(void)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    return data != NULL && (data->allDataEnabled || data->wildEnabled);
}

bool32 Randomizer_IsStarterEnabled(void)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    return data != NULL && (data->allDataEnabled || data->startersEnabled);
}

bool32 Randomizer_IsTrainerEnabled(void)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    return data != NULL && (data->allDataEnabled || data->trainersEnabled);
}

bool32 Randomizer_IsGiftStaticEnabled(void)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    return data != NULL && (data->allDataEnabled || data->giftsStaticEnabled);
}

bool32 Randomizer_AreMovesEnabled(void)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    return data != NULL && (data->allDataEnabled || data->movesEnabled);
}

bool32 Randomizer_AreAbilitiesEnabled(void)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    return data != NULL && (data->allDataEnabled || data->abilitiesEnabled);
}

bool32 Randomizer_AreEvolutionsEnabled(void)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    return data != NULL && (data->allDataEnabled || data->evolutionsEnabled);
}

bool32 Randomizer_AreItemsEnabled(void)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    return data != NULL && (data->allDataEnabled || data->itemsEnabled);
}

bool32 Randomizer_IsAllDataEnabled(void)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    return data != NULL && data->allDataEnabled;
}

void Randomizer_SetWildEnabled(bool32 enabled)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    if (data != NULL)
        data->wildEnabled = enabled;
}

void Randomizer_SetStarterEnabled(bool32 enabled)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    if (data != NULL)
        data->startersEnabled = enabled;
}

void Randomizer_SetTrainerEnabled(bool32 enabled)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    if (data != NULL)
        data->trainersEnabled = enabled;
}

void Randomizer_SetGiftStaticEnabled(bool32 enabled)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    if (data != NULL)
        data->giftsStaticEnabled = enabled;
}

void Randomizer_SetMovesEnabled(bool32 enabled)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    if (data != NULL)
        data->movesEnabled = enabled;
}

void Randomizer_SetAbilitiesEnabled(bool32 enabled)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    if (data != NULL)
        data->abilitiesEnabled = enabled;
}

void Randomizer_SetEvolutionsEnabled(bool32 enabled)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    if (data != NULL)
        data->evolutionsEnabled = enabled;
}

void Randomizer_SetItemsEnabled(bool32 enabled)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    if (data != NULL)
        data->itemsEnabled = enabled;
}

void Randomizer_SetAllDataEnabled(bool32 enabled)
{
    struct RandomizerSaveData *data;

    Randomizer_LoadSave();
    data = GetSaveData();
    if (data != NULL)
    {
        data->allDataEnabled = enabled;
        if (enabled)
        {
            data->wildEnabled = TRUE;
            data->startersEnabled = TRUE;
            data->trainersEnabled = TRUE;
            data->giftsStaticEnabled = TRUE;
            data->movesEnabled = TRUE;
            data->abilitiesEnabled = TRUE;
            data->evolutionsEnabled = TRUE;
            data->itemsEnabled = TRUE;
        }
    }
}

// Returns a stable pseudo-random value without consuming the battle/overworld RNG.
static u32 RandomizerHash(u32 value)
{
    value ^= value >> 16;
    value *= 0x7FEB352D;
    value ^= value >> 15;
    value *= 0x846CA68B;
    value ^= value >> 16;
    return value;
}
static bool32 IsRandomizerEligibleSpecies(enum Species species)
{
    const struct SpeciesInfo *info;

    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return FALSE;

    info = &gSpeciesInfo[species];

    return info->speciesName[0] != EOS
        && !info->isTotem
        && !info->isMegaEvolution
        && !info->isPrimalReversion
        && !info->isUltraBurst
        && !info->isGigantamax
        && !info->isTeraForm
        && !info->isAlolanForm
        && !info->isGalarianForm
        && !info->isHisuianForm
        && !info->isPaldeanForm;
}

static enum Species PickRandomizerSpecies(enum Species originalSpecies, u32 key)
{
    u32 i;

    // HnS randomizes through curated valid-species lists rather than raw
    // SPECIES_* enum slots. The National Pokédex is this expansion's native
    // equivalent: every index resolves to a real base species.
    for (i = 0; i < NATIONAL_DEX_COUNT; i++)
    {
        enum Species species;

        key = RandomizerHash(key + i + 1);
        species = NationalPokedexNumToSpecies(1 + (key % NATIONAL_DEX_COUNT));
        if (species != originalSpecies && IsRandomizerEligibleSpecies(species))
            return species;
    }

    // This is unreachable with a normal expansion species table.
    return SPECIES_MEW;
}

static enum Species PickMonotypeSpecies(enum Species originalSpecies, u32 key)
{
    u32 i;

    for (i = 0; i < NATIONAL_DEX_COUNT; i++)
    {
        enum Species species;

        key = RandomizerHash(key + i + 1);
        species = NationalPokedexNumToSpecies(1 + (key % NATIONAL_DEX_COUNT));
        if (IsRandomizerEligibleSpecies(species) && Nuzlocke_IsSpeciesAllowed(species))
            return species;
    }

    return originalSpecies;
}

enum Move Randomizer_GetMove(enum Move originalMove, u32 key)
{
    u32 i;

    if (!Randomizer_AreMovesEnabled()
     || originalMove == MOVE_NONE
     || originalMove == MOVE_STRUGGLE
     || originalMove >= MOVES_COUNT)
        return originalMove;

    key ^= Randomizer_GetSeed() ^ 0x4D4F5645 ^ originalMove;
    for (i = 0; i < MOVES_COUNT; i++)
    {
        enum Move move;

        key = RandomizerHash(key + i + 1);
        move = 1 + (key % (MOVES_COUNT - 1));
        if (move != MOVE_STRUGGLE
         && gMovesInfo[move].name != NULL
         && gMovesInfo[move].name[0] != EOS
         && gMovesInfo[move].pp != 0
         && gMovesInfo[move].effect != EFFECT_PLACEHOLDER)
            return move;
    }
    return originalMove;
}

enum Ability Randomizer_GetAbility(enum Ability originalAbility, u32 key)
{
    u32 i;

    if (!Randomizer_AreAbilitiesEnabled() || originalAbility == ABILITY_NONE)
        return originalAbility;

    key ^= Randomizer_GetSeed() ^ 0x4142494C ^ originalAbility;
    for (i = 0; i < ABILITIES_COUNT; i++)
    {
        enum Ability ability;

        key = RandomizerHash(key + i + 1);
        ability = 1 + (key % (ABILITIES_COUNT - 1));
        if (gAbilitiesInfo[ability].name[0] != EOS)
            return ability;
    }
    return originalAbility;
}

enum Species Randomizer_GetEvolutionSpecies(enum Species originalSpecies, u32 key)
{
    if (!Randomizer_AreEvolutionsEnabled() || originalSpecies == SPECIES_NONE)
        return originalSpecies;

    return PickRandomizerSpecies(originalSpecies, Randomizer_GetSeed() ^ 0x45564F4C ^ key);
}

enum Item Randomizer_GetItem(enum Item originalItem, u32 key)
{
    u32 i;
    enum Pocket pocket;

    if (!Randomizer_AreItemsEnabled()
     || originalItem == ITEM_NONE
     || originalItem >= ITEMS_COUNT
     || gItemsInfo[originalItem].importance)
        return originalItem;

    pocket = gItemsInfo[originalItem].pocket;
    key ^= Randomizer_GetSeed() ^ 0x4954454D ^ originalItem;
    for (i = 0; i < ITEMS_COUNT; i++)
    {
        enum Item item;

        key = RandomizerHash(key + i + 1);
        item = 1 + (key % (ITEMS_COUNT - 1));
        if (gItemsInfo[item].name != NULL
         && gItemsInfo[item].name[0] != EOS
         && !gItemsInfo[item].importance
         && gItemsInfo[item].pocket == pocket)
            return item;
    }
    return originalItem;
}

enum Species Randomizer_GetWildSpecies(enum Species species)
{
    u32 mapKey = ((u32)(u8)gSaveBlock1Ptr->location.mapGroup << 24)
               | ((u32)(u8)gSaveBlock1Ptr->location.mapNum << 16);

    if (!Randomizer_IsWildEnabled())
        return species;

    return PickRandomizerSpecies(species, Randomizer_GetSeed() ^ mapKey ^ species);
}

enum Species Randomizer_GetStarterSpecies(enum Species originalSpecies, u8 starterId)
{
    if (Nuzlocke_IsActive()
     && ExtendedOptions_Get(EXT_OPT_NUZLOCKE_PRESET) == NUZLOCKE_PRESET_MONOTYPE)
        return PickMonotypeSpecies(originalSpecies, Randomizer_GetSeed() ^ 0x4D4F4E4F ^ starterId);

    if (!Randomizer_IsStarterEnabled())
        return originalSpecies;

    return PickRandomizerSpecies(originalSpecies, Randomizer_GetSeed() ^ 0x53544152 ^ starterId);
}

enum Species Randomizer_GetTrainerSpecies(enum Species originalSpecies, u32 key)
{
    if (!Randomizer_IsTrainerEnabled())
        return originalSpecies;

    return PickRandomizerSpecies(originalSpecies, Randomizer_GetSeed() ^ 0x54524149 ^ key);
}

enum Species Randomizer_GetGiftStaticSpecies(enum Species originalSpecies, u32 key)
{
    if (!Randomizer_IsGiftStaticEnabled())
        return originalSpecies;

    return PickRandomizerSpecies(originalSpecies, Randomizer_GetSeed() ^ 0x47494654 ^ key);
}

#endif // MODULE_RANDOMIZER_ENABLED
