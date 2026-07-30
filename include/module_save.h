#ifndef GUARD_MODULE_SAVE_H
#define GUARD_MODULE_SAVE_H

#include "global.h"
#include "config/module_ids.h"

#define MODULE_SAVE_FORMAT_VERSION 1

void ModuleSave_InitNewSave(void);
void ModuleSave_LoadSave(void);
bool32 ModuleSave_IsValid(void);
void *ModuleSave_GetChunk(enum ModuleId id, u16 *version, u16 *size);
void *ModuleSave_CreateChunk(enum ModuleId id, u16 version, u16 size);
void *ModuleSave_RecreateChunk(enum ModuleId id, u16 version, u16 size);
bool32 ModuleSave_RemoveChunk(enum ModuleId id);

#endif // GUARD_MODULE_SAVE_H
