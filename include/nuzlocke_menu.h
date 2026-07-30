#ifndef GUARD_NUZLOCKE_MENU_H
#define GUARD_NUZLOCKE_MENU_H

#include "main.h"
#include "config/modules.h"

#if MODULE_NUZLOCKE_ENABLED
void StartNuzlockeSetupMenu(MainCallback returnCallback);
void StartNuzlockeSetupMenuWithBack(MainCallback returnCallback, MainCallback backCallback);
void Nuzlocke_ApplyPreset(u8 preset);
#else
static inline void StartNuzlockeSetupMenu(MainCallback returnCallback) { SetMainCallback2(returnCallback); }
static inline void StartNuzlockeSetupMenuWithBack(MainCallback returnCallback, MainCallback backCallback) { SetMainCallback2(returnCallback); }
static inline void Nuzlocke_ApplyPreset(u8 preset) {}
#endif

#endif // GUARD_NUZLOCKE_MENU_H
