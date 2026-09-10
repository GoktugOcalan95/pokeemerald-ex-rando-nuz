#include "global.h"
#include "tera_shards.h"
#include "item.h"
#include "pokemon.h"

bool32 CanApplyTeraShard(struct BoxPokemon *mon, u16 item)
{
    u32 species, type;
    if (item <= ITEM_NONE || item >= ITEMS_COUNT || gItemsInfo[item].sortType != ITEM_TYPE_TERA_SHARD)
        return FALSE;
    species = GetBoxMonData(mon, MON_DATA_SPECIES);
    type = GetItemSecondaryId(item);
    if (species == SPECIES_NONE || species >= NUM_SPECIES || GetBoxMonData(mon, MON_DATA_IS_EGG)
     || !gSpeciesInfo[species].baseHP
     || type == TYPE_NONE || type == TYPE_MYSTERY || type >= NUMBER_OF_MON_TYPES
     || (gSpeciesInfo[species].forceTeraType && gSpeciesInfo[species].forceTeraType != type))
        return FALSE;
    return !GetBoxMonData(mon, MON_DATA_TERA_UNLOCKED) || GetBoxMonData(mon, MON_DATA_TERA_TYPE) != type;
}

bool32 ApplyTeraShard(struct Pokemon *mon, u16 item)
{
    u8 type, unlocked = TRUE;
    if (!CanApplyTeraShard(&mon->box, item) || !RemoveBagItem(item, 1))
        return FALSE;
    type = GetItemSecondaryId(item);
    SetMonData(mon, MON_DATA_TERA_TYPE, &type);
    SetMonData(mon, MON_DATA_TERA_UNLOCKED, &unlocked);
    return TRUE;
}
