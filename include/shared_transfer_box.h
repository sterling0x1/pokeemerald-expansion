#ifndef GUARD_SHARED_TRANSFER_BOX_H
#define GUARD_SHARED_TRANSFER_BOX_H

#include "pokemon.h"

#define SHARED_TRANSFER_BOX_CAPACITY 20
#define SHARED_TRANSFER_BOX_ROGUE_RESERVE_SIZE 2304

struct SharedTransferBox
{
    u32 magic;
    u16 version;
    u16 capacity;
    u32 generation;
    u32 checksum;
    struct BoxPokemon mons[SHARED_TRANSFER_BOX_CAPACITY];
};

bool32 SharedTransferBox_Load(struct SharedTransferBox *box);
bool32 SharedTransferBox_Save(struct SharedTransferBox *box);
void SharedTransferBox_Init(struct SharedTransferBox *box);
bool32 SharedTransferBox_IsSlotOccupied(const struct SharedTransferBox *box, u8 slot);
bool32 SharedTransferBox_Deposit(struct SharedTransferBox *box, u8 slot, const struct BoxPokemon *mon);
bool32 SharedTransferBox_Withdraw(struct SharedTransferBox *box, u8 slot, struct BoxPokemon *mon);
u8 SharedTransferBox_Count(const struct SharedTransferBox *box);

#endif // GUARD_SHARED_TRANSFER_BOX_H
