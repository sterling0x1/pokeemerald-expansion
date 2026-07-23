#include "global.h"
#include "event_data.h"
#include "pokemon.h"
#include "random.h"
#include "randomizer.h"
#include "config/randomizer.h"
#include "constants/pokedex.h"

u32 Randomizer_GetSeed(void)
{
    u32 seed = VarGet(VAR_RANDOMIZER_SEED_LOW)
             | ((u32)VarGet(VAR_RANDOMIZER_SEED_HIGH) << 16);

    if (seed == 0)
        seed = RANDOMIZER_FALLBACK_SEED;

    return seed;
}

void Randomizer_InitNewGameSeed(void)
{
    Randomizer_RerollSeed();
    Randomizer_SetWildEnabled(RANDOMIZER_WILD_POKEMON);
    Randomizer_SetStarterEnabled(RANDOMIZER_STARTERS);
    VarSet(VAR_RANDOMIZER_SETTINGS_INITIALIZED, TRUE);
}

void Randomizer_RerollSeed(void)
{
    u32 oldSeed = Randomizer_GetSeed();
    u32 seed;

    do
    {
        seed = Random32();
    } while (seed == 0 || seed == oldSeed);

    VarSet(VAR_RANDOMIZER_SEED_LOW, seed);
    VarSet(VAR_RANDOMIZER_SEED_HIGH, seed >> 16);
}

bool32 Randomizer_IsWildEnabled(void)
{
    if (VarGet(VAR_RANDOMIZER_SETTINGS_INITIALIZED) != TRUE)
        return FALSE;

    return VarGet(VAR_RANDOMIZER_WILD_ENABLED) != FALSE;
}

bool32 Randomizer_IsStarterEnabled(void)
{
    if (VarGet(VAR_RANDOMIZER_SETTINGS_INITIALIZED) != TRUE)
        return FALSE;

    return VarGet(VAR_RANDOMIZER_STARTERS_ENABLED) != FALSE;
}

void Randomizer_SetWildEnabled(bool32 enabled)
{
    VarSet(VAR_RANDOMIZER_WILD_ENABLED, enabled);
}

void Randomizer_SetStarterEnabled(bool32 enabled)
{
    VarSet(VAR_RANDOMIZER_STARTERS_ENABLED, enabled);
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
    const struct SpeciesInfo *info = &gSpeciesInfo[species];

    return species != SPECIES_NONE
        && species < NUM_SPECIES
        && info->speciesName[0] != 0
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
    if (!Randomizer_IsStarterEnabled())
        return originalSpecies;

    return PickRandomizerSpecies(originalSpecies, Randomizer_GetSeed() ^ 0x53544152 ^ starterId);
}
