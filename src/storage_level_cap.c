#include "global.h"
#include "caps.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "storage_level_cap.h"

static EWRAM_DATA struct
{
    bool8 active;
    bool8 fromBox;
    u8 slot;
    u8 boxId;
    u8 boxPosition;
    struct Pokemon original;
} sStorageLevelCap = {0};

bool32 StorageLevelCap_Begin(bool32 fromBox, u32 boxId, u32 position)
{
    struct BoxPokemon *box;
    u32 slot;

    if (sStorageLevelCap.active || !AreLevelCapsEnabled())
        return FALSE;
    if (fromBox)
    {
        if (boxId >= TOTAL_BOXES_COUNT || position >= IN_BOX_COUNT)
            return FALSE;
        box = GetBoxedMonPtr(boxId, position);
        if (box == NULL || GetBoxMonData(box, MON_DATA_SPECIES) == SPECIES_NONE || GetBoxMonData(box, MON_DATA_IS_EGG))
            return FALSE;
        for (slot = 0; slot < PARTY_SIZE; slot++)
            if (GetMonData(&gParties[B_TRAINER_PLAYER][slot], MON_DATA_SPECIES) == SPECIES_NONE)
                break;
        if (slot == PARTY_SIZE)
            slot = PARTY_SIZE - 1;
        // Keep the box entry intact until the operation finishes.
        sStorageLevelCap.original = gParties[B_TRAINER_PLAYER][slot];
        BoxMonToMon(box, &gParties[B_TRAINER_PLAYER][slot]);
        CalculatePlayerPartyCount();
    }
    else
    {
        slot = position;
        if (slot >= PARTY_SIZE || GetMonData(&gParties[B_TRAINER_PLAYER][slot], MON_DATA_SPECIES) == SPECIES_NONE
         || GetMonData(&gParties[B_TRAINER_PLAYER][slot], MON_DATA_IS_EGG))
            return FALSE;
    }
    sStorageLevelCap.active = TRUE;
    sStorageLevelCap.fromBox = fromBox;
    sStorageLevelCap.slot = slot;
    sStorageLevelCap.boxId = boxId;
    sStorageLevelCap.boxPosition = position;
    return TRUE;
}

void StorageLevelCap_Finish(void)
{
    if (!sStorageLevelCap.active)
        return;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sStorageLevelCap.slot];
    if (GetMonData(mon, MON_DATA_LEVEL) != GetLevelFromMonExp(mon))
        CalculateMonStats(mon);
    if (sStorageLevelCap.fromBox)
    {
        *GetBoxedMonPtr(sStorageLevelCap.boxId, sStorageLevelCap.boxPosition) = gParties[B_TRAINER_PLAYER][sStorageLevelCap.slot].box;
        gParties[B_TRAINER_PLAYER][sStorageLevelCap.slot] = sStorageLevelCap.original;
        CalculatePlayerPartyCount();
    }
    sStorageLevelCap.active = FALSE;
}

u32 StorageLevelCap_GetPartySlot(void)
{
    return sStorageLevelCap.slot;
}

bool32 StorageLevelCap_IsBoxed(void)
{
    return sStorageLevelCap.active && sStorageLevelCap.fromBox;
}

struct Pokemon *GetEvolutionPartyMon(u32 slot)
{
    // Borrowing a slot must not change party-dependent evolution requirements.
    if (StorageLevelCap_IsBoxed() && slot == sStorageLevelCap.slot)
        return &sStorageLevelCap.original;
    return &gParties[B_TRAINER_PLAYER][slot];
}

struct BoxPokemon *StorageLevelCap_GetSplitEvolutionSlot(void)
{
    for (u32 box = 0; box < TOTAL_BOXES_COUNT; box++)
        for (u32 slot = 0; slot < IN_BOX_COUNT; slot++)
            if (GetBoxMonDataAt(box, slot, MON_DATA_SPECIES) == SPECIES_NONE)
                return GetBoxedMonPtr(box, slot);
    return NULL;
}
