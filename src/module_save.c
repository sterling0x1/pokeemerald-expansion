#include "global.h"
#include "module_save.h"

#define MODULE_SAVE_MAGIC 0x4D4F4431

static struct ModuleSaveEntry *FindEntry(enum ModuleId id)
{
    u32 i;

    for (i = 0; i < gSaveBlock3Ptr->moduleSave.entryCount; i++)
    {
        if (gSaveBlock3Ptr->moduleSave.entries[i].id == id)
            return &gSaveBlock3Ptr->moduleSave.entries[i];
    }
    return NULL;
}

bool32 ModuleSave_IsValid(void)
{
    u32 i, j;
    const struct ModuleSaveStore *store = &gSaveBlock3Ptr->moduleSave;

    if (store->magic != MODULE_SAVE_MAGIC
     || store->formatVersion != MODULE_SAVE_FORMAT_VERSION
     || store->entryCount > MODULE_SAVE_MAX_CHUNKS)
        return FALSE;

    for (i = 0; i < store->entryCount; i++)
    {
        const struct ModuleSaveEntry *entry = &store->entries[i];
        u32 end = entry->offset + entry->size;

        if (entry->version == 0 || entry->size == 0 || end > MODULE_SAVE_DATA_CAPACITY)
            return FALSE;
        for (j = i + 1; j < store->entryCount; j++)
        {
            const struct ModuleSaveEntry *other = &store->entries[j];
            u32 otherEnd = other->offset + other->size;

            if (entry->id == other->id
             || (entry->offset < otherEnd && other->offset < end))
                return FALSE;
        }
    }
    return TRUE;
}

void ModuleSave_InitNewSave(void)
{
    memset(&gSaveBlock3Ptr->moduleSave, 0, sizeof(gSaveBlock3Ptr->moduleSave));
    gSaveBlock3Ptr->moduleSave.magic = MODULE_SAVE_MAGIC;
    gSaveBlock3Ptr->moduleSave.formatVersion = MODULE_SAVE_FORMAT_VERSION;
}

void ModuleSave_LoadSave(void)
{
    if (!ModuleSave_IsValid())
        ModuleSave_InitNewSave();
}

void *ModuleSave_GetChunk(enum ModuleId id, u16 *version, u16 *size)
{
    struct ModuleSaveEntry *entry;

    ModuleSave_LoadSave();
    entry = FindEntry(id);
    if (entry == NULL)
        return NULL;
    if (version != NULL)
        *version = entry->version;
    if (size != NULL)
        *size = entry->size;
    return &gSaveBlock3Ptr->moduleSave.data[entry->offset];
}

void *ModuleSave_CreateChunk(enum ModuleId id, u16 version, u16 size)
{
    struct ModuleSaveStore *store = &gSaveBlock3Ptr->moduleSave;
    struct ModuleSaveEntry *entry;
    u32 i;
    u16 offset = 0;

    ModuleSave_LoadSave();
    if (version == 0 || size == 0 || size > MODULE_SAVE_DATA_CAPACITY
     || store->entryCount >= MODULE_SAVE_MAX_CHUNKS || FindEntry(id) != NULL)
        return NULL;

    for (i = 0; i < store->entryCount; i++)
        offset = max(offset, store->entries[i].offset + store->entries[i].size);
    if (offset + size > MODULE_SAVE_DATA_CAPACITY)
        return NULL;

    entry = &store->entries[store->entryCount++];
    entry->id = id;
    entry->version = version;
    entry->offset = offset;
    entry->size = size;
    memset(&store->data[offset], 0, size);
    return &store->data[offset];
}

bool32 ModuleSave_RemoveChunk(enum ModuleId id)
{
    struct ModuleSaveStore *store = &gSaveBlock3Ptr->moduleSave;
    struct ModuleSaveEntry *entry;
    u32 i;
    u16 offset;
    u16 size;

    ModuleSave_LoadSave();
    entry = FindEntry(id);
    if (entry == NULL)
        return FALSE;

    offset = entry->offset;
    size = entry->size;
    memmove(&store->data[offset], &store->data[offset + size], MODULE_SAVE_DATA_CAPACITY - offset - size);
    memset(&store->data[MODULE_SAVE_DATA_CAPACITY - size], 0, size);

    for (i = 0; i < store->entryCount; i++)
    {
        if (store->entries[i].offset > offset)
            store->entries[i].offset -= size;
    }

    i = entry - store->entries;
    for (; i + 1 < store->entryCount; i++)
        store->entries[i] = store->entries[i + 1];
    memset(&store->entries[--store->entryCount], 0, sizeof(store->entries[0]));
    return TRUE;
}

void *ModuleSave_RecreateChunk(enum ModuleId id, u16 version, u16 size)
{
    ModuleSave_RemoveChunk(id);
    return ModuleSave_CreateChunk(id, version, size);
}
