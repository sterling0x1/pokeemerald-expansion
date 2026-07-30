#include "global.h"
#include "battle.h"
#include "extended_options.h"
#include "data.h"
#include "nuzlocke.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "constants/battle.h"
#include "constants/pokemon.h"

#define NUZLOCKE_SAVE_MAGIC 0x4E555A31

static EWRAM_DATA bool8 sCanCatchPartyMon[PARTY_SIZE];
static EWRAM_DATA bool8 sReplaceAreaCatch[PARTY_SIZE];
static EWRAM_DATA u8 sCurrentEncounterMapSection;
static EWRAM_DATA bool8 sNextEncounterIsStatic;
static EWRAM_DATA bool8 sGiftConsumesLocation;
static EWRAM_DATA bool8 sGiftUsesStaticLocation;
static EWRAM_DATA u8 sGiftMapSection;

static bool32 IsSpeciesCaught(enum Species species)
{
    return GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT);
}

static bool32 IsEvolutionFamilyCaught(enum Species species, u32 depth)
{
    const struct Evolution *evolutions;
    u32 i;

    if (IsSpeciesCaught(species))
        return TRUE;
    if (depth >= 10)
        return FALSE;

    evolutions = GetSpeciesEvolutions(species);
    if (evolutions == NULL)
        return FALSE;
    for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
    {
        enum Species target = evolutions[i].targetSpecies;

        if (target != species && IsEvolutionFamilyCaught(target, depth + 1))
            return TRUE;
    }
    return FALSE;
}

static bool32 IsDuplicateSpecies(enum Species species)
{
    enum Species familyRoot = species;
    u8 clause = ExtendedOptions_Get(EXT_OPT_NUZLOCKE_DUPES);

    if (clause == NUZLOCKE_DUPES_OFF)
        return FALSE;
    if (IsSpeciesCaught(species))
        return TRUE;

    if (clause == NUZLOCKE_DUPES_FAMILY)
    {
        while (GetSpeciesPreEvolution(familyRoot) != SPECIES_NONE)
            familyRoot = GetSpeciesPreEvolution(familyRoot);
        return IsEvolutionFamilyCaught(familyRoot, 0);
    }

    return FALSE;
}

static void RestrictPreviousAreaCatch(u8 mapSection)
{
    u32 i, box, position;
    u8 value = TRUE;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];

        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE
         && GetMonData(mon, MON_DATA_MET_LOCATION) == mapSection
         && !GetMonData(mon, MON_DATA_IS_EGG))
            SetMonData(mon, MON_DATA_NUZLOCKE_RESTRICTED, &value);
    }

    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
    {
        for (position = 0; position < IN_BOX_COUNT; position++)
        {
            struct BoxPokemon *boxMon = GetBoxedMonPtr(box, position);

            if (GetBoxMonData(boxMon, MON_DATA_SPECIES) != SPECIES_NONE
             && GetBoxMonData(boxMon, MON_DATA_MET_LOCATION) == mapSection
             && !GetBoxMonData(boxMon, MON_DATA_IS_EGG))
                SetBoxMonData(boxMon, MON_DATA_NUZLOCKE_RESTRICTED, &value);
        }
    }
}

void Nuzlocke_InitRunData(void)
{
    memset(&gSaveBlock3Ptr->nuzlocke, 0, sizeof(gSaveBlock3Ptr->nuzlocke));
    gSaveBlock3Ptr->nuzlocke.magic = NUZLOCKE_SAVE_MAGIC;
    gSaveBlock3Ptr->nuzlocke.monotype = NUZLOCKE_NO_TYPE;
}

void Nuzlocke_EnsureRunData(void)
{
    if (gSaveBlock3Ptr->nuzlocke.magic != NUZLOCKE_SAVE_MAGIC)
        Nuzlocke_InitRunData();
}

bool32 Nuzlocke_IsActive(void)
{
    return ExtendedOptions_Get(EXT_OPT_NUZLOCKE) && !Nuzlocke_HasRunEnded();
}

bool32 Nuzlocke_IsLocationUsed(bool32 isStaticEncounter, u8 mapSection)
{
    const u8 *flags;

    Nuzlocke_EnsureRunData();
    flags = isStaticEncounter
          ? gSaveBlock3Ptr->nuzlocke.staticEncounterLocations
          : gSaveBlock3Ptr->nuzlocke.wildEncounterLocations;
    return (flags[mapSection / 8] & (1u << (mapSection % 8))) != 0;
}

void Nuzlocke_SetLocationUsed(bool32 isStaticEncounter, u8 mapSection)
{
    u8 *flags;

    Nuzlocke_EnsureRunData();
    flags = isStaticEncounter
          ? gSaveBlock3Ptr->nuzlocke.staticEncounterLocations
          : gSaveBlock3Ptr->nuzlocke.wildEncounterLocations;
    flags[mapSection / 8] |= 1u << (mapSection % 8);
}

u8 Nuzlocke_GetMonotype(void)
{
    Nuzlocke_EnsureRunData();
    return gSaveBlock3Ptr->nuzlocke.monotype;
}

void Nuzlocke_SetMonotype(u8 type)
{
    Nuzlocke_EnsureRunData();
    gSaveBlock3Ptr->nuzlocke.monotype = type;
}

bool32 Nuzlocke_AreSettingsLocked(void)
{
    Nuzlocke_EnsureRunData();
    return gSaveBlock3Ptr->nuzlocke.settingsLocked;
}

void Nuzlocke_LockSettings(void)
{
    Nuzlocke_EnsureRunData();
    gSaveBlock3Ptr->nuzlocke.battleStyle = gSaveBlock2Ptr->optionsBattleStyle;
    gSaveBlock3Ptr->nuzlocke.settingsLocked = TRUE;
}

u8 Nuzlocke_GetBattleStyle(void)
{
    Nuzlocke_EnsureRunData();
    return gSaveBlock3Ptr->nuzlocke.battleStyle;
}

bool32 Nuzlocke_HasRunEnded(void)
{
    Nuzlocke_EnsureRunData();
    return gSaveBlock3Ptr->nuzlocke.runEnded;
}

void Nuzlocke_EndRun(void)
{
    Nuzlocke_EnsureRunData();
    gSaveBlock3Ptr->nuzlocke.runEnded = TRUE;
}

void Nuzlocke_BeginWildEncounter(void)
{
    bool32 isStaticEncounter = sNextEncounterIsStatic;
    bool32 locationUsed;
    bool32 consumeLocation = FALSE;
    bool32 ignoreLocation = FALSE;
    u8 mapSection;
    u8 giftRule;
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        sCanCatchPartyMon[i] = TRUE;
        sReplaceAreaCatch[i] = FALSE;
    }
    sNextEncounterIsStatic = FALSE;
    if (!Nuzlocke_IsActive()
     || ExtendedOptions_Get(EXT_OPT_NUZLOCKE_ENCOUNTERS) == NUZLOCKE_ENCOUNTERS_OFF
     || (gBattleTypeFlags & (BATTLE_TYPE_TRAINER | BATTLE_TYPE_CATCH_TUTORIAL
                           | BATTLE_TYPE_FIRST_BATTLE | BATTLE_TYPE_SAFARI
                           | BATTLE_TYPE_PYRAMID | BATTLE_TYPE_PIKE | BATTLE_TYPE_GHOST)))
        return;

    isStaticEncounter |= (gBattleTypeFlags & BATTLE_TYPE_LEGENDARY) != 0;
    giftRule = ExtendedOptions_Get(EXT_OPT_NUZLOCKE_GIFTS);
    if (isStaticEncounter && giftRule == NUZLOCKE_GIFTS_FREE)
        ignoreLocation = TRUE;
    if (isStaticEncounter && giftRule == NUZLOCKE_GIFTS_COUNT)
        isStaticEncounter = FALSE;

    mapSection = GetCurrentRegionMapSectionId();
    sCurrentEncounterMapSection = mapSection;
    locationUsed = !ignoreLocation && Nuzlocke_IsLocationUsed(isStaticEncounter, mapSection);

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][i];
        enum Species species = GetMonData(mon, MON_DATA_SPECIES);
        u8 shinyClause = ExtendedOptions_Get(EXT_OPT_NUZLOCKE_SHINY_CLAUSE);
        bool32 isShiny;

        if (species == SPECIES_NONE)
            continue;

        isShiny = IsMonShiny(mon);
        if (isShiny && shinyClause == NUZLOCKE_SHINY_COLLECTION)
        {
            u8 value = TRUE;
            SetMonData(mon, MON_DATA_NUZLOCKE_RESTRICTED, &value);
            sCanCatchPartyMon[i] = TRUE;
            continue;
        }
        if (isShiny && shinyClause == NUZLOCKE_SHINY_FREE)
        {
            sCanCatchPartyMon[i] = TRUE;
            continue;
        }
        if (isShiny && shinyClause == NUZLOCKE_SHINY_REPLACE && locationUsed)
        {
            if (!isStaticEncounter)
                sReplaceAreaCatch[i] = TRUE;
            sCanCatchPartyMon[i] = TRUE;
            continue;
        }

        sCanCatchPartyMon[i] = !locationUsed
                            && Nuzlocke_IsSpeciesAllowed(species)
                            && !IsDuplicateSpecies(species);
        if (sCanCatchPartyMon[i] && !ignoreLocation)
            consumeLocation = TRUE;
    }

    if (consumeLocation)
        Nuzlocke_SetLocationUsed(isStaticEncounter, mapSection);
}

void Nuzlocke_SetNextEncounterStatic(void)
{
    sNextEncounterIsStatic = TRUE;
}

bool32 Nuzlocke_CanCatchCurrentEncounter(enum BattlerId battler)
{
    u8 partyIndex;

    if (!Nuzlocke_IsActive())
        return TRUE;
    partyIndex = gBattlerPartyIndexes[battler];
    return partyIndex < PARTY_SIZE && sCanCatchPartyMon[partyIndex];
}

void Nuzlocke_CommitCaughtMon(enum BattlerId battler)
{
    u8 partyIndex = gBattlerPartyIndexes[battler];

    if (partyIndex < PARTY_SIZE && sReplaceAreaCatch[partyIndex])
    {
        RestrictPreviousAreaCatch(sCurrentEncounterMapSection);
        sReplaceAreaCatch[partyIndex] = FALSE;
    }
}

bool32 Nuzlocke_IsSpeciesAllowed(enum Species species)
{
    u8 type;

    if (!Nuzlocke_IsActive()
     || ExtendedOptions_Get(EXT_OPT_NUZLOCKE_PRESET) != NUZLOCKE_PRESET_MONOTYPE)
        return TRUE;

    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return FALSE;

    type = Nuzlocke_GetMonotype();
    return gSpeciesInfo[species].types[0] == type || gSpeciesInfo[species].types[1] == type;
}

bool32 Nuzlocke_IsBoxMonUsable(struct BoxPokemon *boxMon)
{
    if (!Nuzlocke_IsActive())
        return TRUE;
    return !GetBoxMonData(boxMon, MON_DATA_NUZLOCKE_DEAD)
        && !GetBoxMonData(boxMon, MON_DATA_NUZLOCKE_RESTRICTED)
        && Nuzlocke_IsSpeciesAllowed(GetBoxMonData(boxMon, MON_DATA_SPECIES));
}

bool32 Nuzlocke_IsMonUsable(struct Pokemon *mon)
{
    return Nuzlocke_IsBoxMonUsable(&mon->box);
}

static void ProcessFaintedParty(bool32 checkBattleType)
{
    u32 i;
    u8 value = TRUE;
    bool32 removeMon;

    if (!Nuzlocke_IsActive())
        return;

    if (checkBattleType
     && (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK
                          | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_TRAINER_HILL
                          | BATTLE_TYPE_INGAME_PARTNER | BATTLE_TYPE_TOWER_LINK_MULTI
                          | BATTLE_TYPE_FIRST_BATTLE | BATTLE_TYPE_CATCH_TUTORIAL
                          | BATTLE_TYPE_SAFARI | BATTLE_TYPE_SECRET_BASE)))
        return;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];

        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE
         || GetMonData(mon, MON_DATA_IS_EGG)
         || GetMonData(mon, MON_DATA_HP) != 0
         || GetMonData(mon, MON_DATA_NUZLOCKE_DEAD))
            continue;

        SetMonData(mon, MON_DATA_NUZLOCKE_DEAD, &value);
        removeMon = ExtendedOptions_Get(EXT_OPT_NUZLOCKE_PERMADEATH) == NUZLOCKE_PERMADEATH_RELEASE;
        if (!removeMon)
            removeMon = CopyMonToPC(mon) == MON_GIVEN_TO_PC;
        if (removeMon)
            ZeroMonData(mon);
    }

    CompactPartySlots();
    CalculatePlayerPartyCount();
}

void Nuzlocke_ProcessFaintedParty(void)
{
    ProcessFaintedParty(TRUE);
}

void Nuzlocke_ProcessFieldFaints(void)
{
    ProcessFaintedParty(FALSE);
}

void Nuzlocke_PrepareGiftMon(struct Pokemon *mon)
{
    bool32 isStaticEncounter;
    bool32 restricted = FALSE;
    u8 mapSection;
    u8 value = TRUE;
    enum Species species;

    sGiftConsumesLocation = FALSE;
    if (!Nuzlocke_IsActive())
        return;

    species = GetMonData(mon, MON_DATA_SPECIES);
    if (!Nuzlocke_IsSpeciesAllowed(species) || IsDuplicateSpecies(species))
        restricted = TRUE;

    switch (ExtendedOptions_Get(EXT_OPT_NUZLOCKE_GIFTS))
    {
    case NUZLOCKE_GIFTS_FREE:
        break;
    case NUZLOCKE_GIFTS_COUNT:
        isStaticEncounter = FALSE;
        mapSection = GetCurrentRegionMapSectionId();
        if (Nuzlocke_IsLocationUsed(isStaticEncounter, mapSection))
            restricted = TRUE;
        else if (!restricted)
        {
            sGiftConsumesLocation = TRUE;
            sGiftUsesStaticLocation = isStaticEncounter;
            sGiftMapSection = mapSection;
        }
        break;
    case NUZLOCKE_GIFTS_SEPARATE:
        isStaticEncounter = TRUE;
        mapSection = GetCurrentRegionMapSectionId();
        if (Nuzlocke_IsLocationUsed(isStaticEncounter, mapSection))
            restricted = TRUE;
        else if (!restricted)
        {
            sGiftConsumesLocation = TRUE;
            sGiftUsesStaticLocation = isStaticEncounter;
            sGiftMapSection = mapSection;
        }
        break;
    }

    if (GetMonData(mon, MON_DATA_IS_SHINY)
     && ExtendedOptions_Get(EXT_OPT_NUZLOCKE_SHINY_CLAUSE) == NUZLOCKE_SHINY_COLLECTION)
        restricted = TRUE;

    if (restricted)
        SetMonData(mon, MON_DATA_NUZLOCKE_RESTRICTED, &value);
}

void Nuzlocke_CommitGiftMon(bool32 wasGiven)
{
    if (wasGiven && sGiftConsumesLocation)
        Nuzlocke_SetLocationUsed(sGiftUsesStaticLocation, sGiftMapSection);
    sGiftConsumesLocation = FALSE;
}

bool32 Nuzlocke_TryRestoreUsablePartyFromPC(void)
{
    u32 i, box, position;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES) != SPECIES_NONE
         && Nuzlocke_IsMonUsable(&gParties[B_TRAINER_PLAYER][i]))
            return TRUE;
    }

    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
    {
        for (position = 0; position < IN_BOX_COUNT; position++)
        {
            struct BoxPokemon *boxMon = GetBoxedMonPtr(box, position);

            if (GetBoxMonData(boxMon, MON_DATA_SPECIES) != SPECIES_NONE
             && Nuzlocke_IsBoxMonUsable(boxMon))
            {
                BoxMonToMon(boxMon, &gParties[B_TRAINER_PLAYER][0]);
                ZeroBoxMonData(boxMon);
                CompactPartySlots();
                CalculatePlayerPartyCount();
                return TRUE;
            }
        }
    }

    return FALSE;
}
