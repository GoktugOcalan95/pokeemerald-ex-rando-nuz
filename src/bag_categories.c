#include "global.h"
#include "bag_categories.h"
#include "boss_rewards.h"
#include "item.h"
#include "event_data.h"
#include "string_util.h"
#include "strings.h"
#include "slateport_shops.h"

// Gameplay pockets retain their original meaning for berries, machines, and keys.
enum BagPocketId GetBagStoragePocket(enum Item item)
{
    if (item <= ITEM_NONE || item >= ITEMS_COUNT || gItemsInfo[item].name == NULL)
        return BAG_POCKETS_COUNT;
    switch (GetItemPocket(item))
    {
    case POCKET_KEY_ITEMS: return BAG_KEY_ITEMS;
    case POCKET_TM_HM: return BAG_TM_HM;
    case POCKET_BERRIES: return BAG_BERRIES;
    case POCKET_POKE_BALLS: return BAG_POKE_BALLS;
    default: break;
    }
    if (IsAbilityCustomizationItem(item))
        return BAG_TRAINING;
    if (item == ITEM_HONEY || item == ITEM_BLACK_FLUTE || item == ITEM_WHITE_FLUTE)
        return BAG_OTHER_ITEMS;
    if (IsFunctionalEvolutionOrFormItem(item))
        return BAG_EVOLUTION_FORMS;
    switch (gItemsInfo[item].sortType)
    {
    case ITEM_TYPE_LEVEL_UP_ITEM:
    case ITEM_TYPE_NATURE_MINT:
    case ITEM_TYPE_STAT_BOOST_DRINK:
    case ITEM_TYPE_STAT_BOOST_FEATHER:
    case ITEM_TYPE_STAT_BOOST_MOCHI:
        return BAG_TRAINING;
    case ITEM_TYPE_HEALTH_RECOVERY:
    case ITEM_TYPE_STATUS_RECOVERY:
    case ITEM_TYPE_PP_RECOVERY:
    case ITEM_TYPE_FLUTE:
        return BAG_MEDICINE;
    case ITEM_TYPE_BATTLE_ITEM:
    case ITEM_TYPE_X_ITEM:
        return BAG_BATTLE_ITEMS;
    case ITEM_TYPE_EVOLUTION_STONE:
    case ITEM_TYPE_EVOLUTION_ITEM:
    case ITEM_TYPE_MEGA_STONE:
    case ITEM_TYPE_Z_CRYSTAL:
    case ITEM_TYPE_TERA_SHARD:
    case ITEM_TYPE_PLATE:
    case ITEM_TYPE_MEMORY:
    case ITEM_TYPE_DRIVE:
    case ITEM_TYPE_NECTAR:
    case ITEM_TYPE_FOSSIL:
        return BAG_EVOLUTION_FORMS;
    case ITEM_TYPE_SPECIAL_HELD_ITEM:
    case ITEM_TYPE_HELD_ITEM:
    case ITEM_TYPE_TYPE_BOOST_HELD_ITEM:
    case ITEM_TYPE_CONTEST_HELD_ITEM:
    case ITEM_TYPE_EV_BOOST_HELD_ITEM:
    case ITEM_TYPE_GEM:
    case ITEM_TYPE_INCENSE:
        return BAG_HELD_ITEMS;
    case ITEM_TYPE_SELLABLE:
    case ITEM_TYPE_RELIC:
    case ITEM_TYPE_SHARD:
        return BAG_TREASURES;
    default:
        return BAG_OTHER_ITEMS;
    }
}

void BufferItemBagPocketName(void)
{
    enum BagPocketId pocket = GetBagStoragePocket(gSpecialVar_0x8006);
    if (pocket < BAG_POCKETS_COUNT)
        StringCopy(gStringVar3, gPocketNamesStringsTable[pocket]);
}
