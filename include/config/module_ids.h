#ifndef GUARD_CONFIG_MODULE_IDS_H
#define GUARD_CONFIG_MODULE_IDS_H

// Project-owned stable IDs. The generic module manager does not assign these;
// an integration build extends this list when it adds another module.
enum ModuleId
{
    MODULE_ID_RANDOMIZER,
    MODULE_ID_NUZLOCKE,
    MODULE_ID_CHEATS,
    MODULE_ID_QOL,
    MODULE_ID_BATTLE_PACING,
    MODULE_ID_COUNT,
};

#endif // GUARD_CONFIG_MODULE_IDS_H
