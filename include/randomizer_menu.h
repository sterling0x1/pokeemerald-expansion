#ifndef GUARD_RANDOMIZER_MENU_H
#define GUARD_RANDOMIZER_MENU_H

#include "main.h"
#include "config/modules.h"

#if MODULE_RANDOMIZER_ENABLED
void StartRandomizerSetupMenu(MainCallback returnCallback);
void StartRandomizerSetupMenuWithBack(MainCallback returnCallback, MainCallback backCallback);
#else
static inline void StartRandomizerSetupMenu(MainCallback returnCallback)
{
    SetMainCallback2(returnCallback);
}

static inline void StartRandomizerSetupMenuWithBack(MainCallback returnCallback, MainCallback backCallback)
{
    (void)backCallback;
    SetMainCallback2(returnCallback);
}
#endif

#endif // GUARD_RANDOMIZER_MENU_H
