#include "global.h"
#include "pokemon.h"
#include "randomizer.h"
#include "config/randomizer.h"

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

static enum Species PickRandomizerSpecies(u32 key)
{
    u32 i;

    for (i = 0; i < NUM_SPECIES; i++)
    {
        enum Species species;

        key = RandomizerHash(key + i + 1);
        species = 1 + (key % (NUM_SPECIES - 1));
        if (IsRandomizerEligibleSpecies(species))
            return species;
    }

    // This is unreachable with a normal expansion species table.
    return SPECIES_MEW;
}

enum Species Randomizer_GetWildSpecies(enum Species species)
{
#if RANDOMIZER_WILD_POKEMON
    u32 mapKey = ((u32)(u8)gSaveBlock1Ptr->location.mapGroup << 24)
               | ((u32)(u8)gSaveBlock1Ptr->location.mapNum << 16);

    return PickRandomizerSpecies(RANDOMIZER_SEED ^ mapKey ^ species);
#else
    return species;
#endif
}

enum Species Randomizer_GetStarterSpecies(enum Species originalSpecies, u8 starterId)
{
#if RANDOMIZER_STARTERS
    return PickRandomizerSpecies(RANDOMIZER_SEED ^ 0x53544152 ^ starterId);
#else
    return originalSpecies;
#endif
}
