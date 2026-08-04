#include "global.h"
#include "malloc.h"
#include "save.h"
#include "shared_transfer_box.h"

#define SHARED_TRANSFER_BOX_MAGIC 0x58425453 // "STBX"
#define SHARED_TRANSFER_BOX_VERSION 1
#define SPECIAL_SECTOR_PAYLOAD_SIZE SECTOR_COUNTER_OFFSET

STATIC_ASSERT(sizeof(struct SharedTransferBox) + SHARED_TRANSFER_BOX_ROGUE_RESERVE_SIZE <= SPECIAL_SECTOR_PAYLOAD_SIZE,
              SharedTransferBoxLeavesRogueReserve);

#if MODULE_SHARED_TRANSFER_BOX_ENABLED
static u32 CalculateTransferBoxChecksum(const struct SharedTransferBox *box)
{
    const u8 *data = (const u8 *)box;
    u32 checksum = 2166136261;
    u32 i;

    for (i = 0; i < sizeof(*box); i++)
    {
        if (i >= offsetof(struct SharedTransferBox, checksum)
         && i < offsetof(struct SharedTransferBox, checksum) + sizeof(box->checksum))
            continue;
        checksum ^= data[i];
        checksum *= 16777619;
    }
    return checksum;
}
#endif

void SharedTransferBox_Init(struct SharedTransferBox *box)
{
    memset(box, 0, sizeof(*box));
    box->magic = SHARED_TRANSFER_BOX_MAGIC;
    box->version = SHARED_TRANSFER_BOX_VERSION;
    box->capacity = SHARED_TRANSFER_BOX_CAPACITY;
}

bool32 SharedTransferBox_Load(struct SharedTransferBox *box)
{
#if MODULE_SHARED_TRANSFER_BOX_ENABLED
    u8 *sectorData = AllocZeroed(SPECIAL_SECTOR_PAYLOAD_SIZE);
    bool32 valid = FALSE;

    if (sectorData != NULL
     && TryReadSpecialSaveSector(SECTOR_ID_RECORDED_BATTLE, sectorData) == SAVE_STATUS_OK)
    {
        memcpy(box, sectorData, sizeof(*box));
        valid = box->magic == SHARED_TRANSFER_BOX_MAGIC
             && box->version == SHARED_TRANSFER_BOX_VERSION
             && box->capacity == SHARED_TRANSFER_BOX_CAPACITY
             && box->checksum == CalculateTransferBoxChecksum(box);
    }
    Free(sectorData);
    if (!valid)
        SharedTransferBox_Init(box);
    return valid;
#else
    SharedTransferBox_Init(box);
    return FALSE;
#endif
}

bool32 SharedTransferBox_Save(struct SharedTransferBox *box)
{
#if MODULE_SHARED_TRANSFER_BOX_ENABLED
    u8 *sectorData = AllocZeroed(SPECIAL_SECTOR_PAYLOAD_SIZE);
    bool32 success = FALSE;

    if (sectorData == NULL)
        return FALSE;
    box->magic = SHARED_TRANSFER_BOX_MAGIC;
    box->version = SHARED_TRANSFER_BOX_VERSION;
    box->capacity = SHARED_TRANSFER_BOX_CAPACITY;
    box->generation++;
    box->checksum = CalculateTransferBoxChecksum(box);
    memcpy(sectorData, box, sizeof(*box));
    success = TryWriteSpecialSaveSector(SECTOR_ID_RECORDED_BATTLE, sectorData) == SAVE_STATUS_OK;
    Free(sectorData);
    return success;
#else
    return FALSE;
#endif
}

bool32 SharedTransferBox_IsSlotOccupied(const struct SharedTransferBox *box, u8 slot)
{
    return slot < SHARED_TRANSFER_BOX_CAPACITY && box->mons[slot].hasSpecies;
}

bool32 SharedTransferBox_Deposit(struct SharedTransferBox *box, u8 slot, const struct BoxPokemon *mon)
{
    if (slot >= SHARED_TRANSFER_BOX_CAPACITY || box->mons[slot].hasSpecies || !mon->hasSpecies)
        return FALSE;
    box->mons[slot] = *mon;
    return TRUE;
}

bool32 SharedTransferBox_Withdraw(struct SharedTransferBox *box, u8 slot, struct BoxPokemon *mon)
{
    if (!SharedTransferBox_IsSlotOccupied(box, slot))
        return FALSE;
    *mon = box->mons[slot];
    ZeroBoxMonData(&box->mons[slot]);
    return TRUE;
}

u8 SharedTransferBox_Count(const struct SharedTransferBox *box)
{
    u8 count = 0;
    u8 i;

    for (i = 0; i < SHARED_TRANSFER_BOX_CAPACITY; i++)
        if (SharedTransferBox_IsSlotOccupied(box, i))
            count++;
    return count;
}
