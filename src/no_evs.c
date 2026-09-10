#include "global.h"
#include "no_evs.h"
#include "event_data.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"

void NormalizeMonEVs(struct Pokemon *mon)
{
    if (NormalizeBoxMonEVs(&mon->box))
        CalculateMonStats(mon);
}

void NormalizeStoredMonEVs(void)
{
    if (!FlagGet(FLAG_RUN_RULE_NO_EV_GAIN))
        return;
    for (u32 slot = 0; slot < PARTY_SIZE; slot++)
    {
        NormalizeMonEVs(&gParties[B_TRAINER_PLAYER][slot]);
        NormalizeMonEVs(&gSaveBlock1Ptr->playerParty[slot]);
    }
    for (u32 box = 0; box < TOTAL_BOXES_COUNT; box++)
        for (u32 slot = 0; slot < IN_BOX_COUNT; slot++)
            NormalizeBoxMonEVs(&gPokemonStoragePtr->boxes[box][slot]);
    for (u32 slot = 0; slot < MAX_FUSION_STORAGE; slot++)
        NormalizeMonEVs(&gPokemonStoragePtr->fusions[slot]);
    for (u32 slot = 0; slot < ARRAY_COUNT(gSaveBlock1Ptr->daycare.mons); slot++)
        NormalizeBoxMonEVs(&gSaveBlock1Ptr->daycare.mons[slot].mon);
#if IS_FRLG
    NormalizeBoxMonEVs(&gSaveBlock1Ptr->route5DayCareMon.mon);
#endif
}

bool32 IsEVRelatedItem(u16 item)
{
    return (item >= ITEM_HP_UP && item <= ITEM_CARBOS)
        || (item >= ITEM_HEALTH_FEATHER && item <= ITEM_SWIFT_FEATHER)
        || (item >= ITEM_MACHO_BRACE && item <= ITEM_POWER_ANKLET)
        || (item >= ITEM_HEALTH_MOCHI && item <= ITEM_GLIMMERING_CHARM);
}

bool32 IsItemAllowedByNoEVs(u16 item)
{
    return !FlagGet(FLAG_RUN_RULE_NO_EV_GAIN) || !IsEVRelatedItem(item);
}
