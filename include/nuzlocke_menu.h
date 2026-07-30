#ifndef GUARD_NUZLOCKE_MENU_H
#define GUARD_NUZLOCKE_MENU_H

#include "main.h"

void StartNuzlockeSetupMenu(MainCallback returnCallback);
void StartNuzlockeSetupMenuWithBack(MainCallback returnCallback, MainCallback backCallback);
void Nuzlocke_ApplyPreset(u8 preset);

#endif // GUARD_NUZLOCKE_MENU_H
