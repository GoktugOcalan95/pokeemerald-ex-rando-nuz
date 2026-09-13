#include "global.h"
#include "slateport_shops.h"
#include "battle_z_move.h"
#include "event_data.h"
#include "item.h"
#include "no_evs.h"
#include "pokemon.h"
#include "string_util.h"

static EWRAM_DATA u16 sStockCategories[ITEMS_COUNT] = {0};
static EWRAM_DATA bool8 sStockReady = FALSE;

static bool32 IsAvailableSpecies(u32 species)
{
    return species > SPECIES_NONE && species < NUM_SPECIES && species != SPECIES_EGG && IsSpeciesEnabled(species);
}

static void AddStockItem(u32 item, u32 category)
{
    if (item > ITEM_NONE && item < ITEMS_COUNT)
        sStockCategories[item] |= 1 << category;
}

static void AddEvolutionItem(u32 item)
{
    if (item > ITEM_NONE && item < ITEMS_COUNT)
        AddStockItem(item, gItemsInfo[item].sortType == ITEM_TYPE_EVOLUTION_STONE ? SLATEPORT_SHOP_TM : SLATEPORT_SHOP_EVOLUTION);
}

static void AddFormItem(u32 item)
{
    if (item <= ITEM_NONE || item >= ITEMS_COUNT)
        return;
    if (gItemsInfo[item].sortType == ITEM_TYPE_Z_CRYSTAL)
        return;
    AddStockItem(item, SLATEPORT_SHOP_FORMS);
    if (gItemsInfo[item].sortType == ITEM_TYPE_NECTAR)
        AddStockItem(item, SLATEPORT_SHOP_EVOLUTION);
    else if (gItemsInfo[item].sortType != ITEM_TYPE_PLATE
          && gItemsInfo[item].sortType != ITEM_TYPE_MEMORY
          && gItemsInfo[item].sortType != ITEM_TYPE_DRIVE)
        AddStockItem(item, SLATEPORT_SHOP_TM);
}

static void InitStockCategories(void)
{
    if (sStockReady)
        return;
    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        if (!IsAvailableSpecies(species))
            continue;
        const struct Evolution *evo = GetSpeciesEvolutions(species);
        for (u32 i = 0; evo != NULL && evo[i].method != EVOLUTIONS_END; i++)
        {
            if (!IsAvailableSpecies(evo[i].targetSpecies) || evo[i].method == EVO_NONE || evo[i].method == EVO_SPLIT_FROM_EVO)
                continue;
            if (evo[i].method == EVO_ITEM)
                AddEvolutionItem(evo[i].param);
            const struct EvolutionParam *params = evo[i].params;
            for (u32 j = 0; params != NULL && params[j].condition != CONDITIONS_END; j++)
                if (params[j].condition == IF_HOLD_ITEM || params[j].condition == IF_BAG_ITEM_COUNT)
                    AddEvolutionItem(params[j].arg1);
        }
        const struct FormChange *form = GetSpeciesFormChanges(species);
        for (u32 i = 0; form != NULL && form[i].method != FORM_CHANGE_TERMINATOR; i++)
        {
            if (!IsAvailableSpecies(form[i].targetSpecies))
                continue;
            switch (form[i].method)
            {
            case FORM_CHANGE_BATTLE_MEGA_EVOLUTION_ITEM:
                AddStockItem(form[i].param1, SLATEPORT_SHOP_MEGA);
                break;
            case FORM_CHANGE_ITEM_HOLD:
            case FORM_CHANGE_ITEM_USE:
            case FORM_CHANGE_ITEM_USE_MULTICHOICE:
            case FORM_CHANGE_BEGIN_BATTLE:
            case FORM_CHANGE_END_BATTLE:
            case FORM_CHANGE_BATTLE_PRIMAL_REVERSION:
            case FORM_CHANGE_BATTLE_ULTRA_BURST:
                AddFormItem(form[i].param1);
                break;
            default:
                break;
            }
        }
        const struct Fusion *fusion = gFusionTablePointers[species];
        for (u32 i = 0; fusion != NULL && fusion[i].fusionStorageIndex != FUSION_TERMINATOR; i++)
            if (IsAvailableSpecies(fusion[i].targetSpecies1) && IsAvailableSpecies(fusion[i].targetSpecies2)
             && IsAvailableSpecies(fusion[i].fusingIntoMon))
                AddFormItem(fusion[i].itemId);
    }
    for (u32 item = 1; item < ITEMS_COUNT; item++)
    {
        switch (gItemsInfo[item].sortType)
        {
        case ITEM_TYPE_STAT_BOOST_DRINK:
        case ITEM_TYPE_STAT_BOOST_FEATHER:
            if (IsEVRelatedItem(item))
                AddStockItem(item, SLATEPORT_SHOP_ENERGY);
            break;
        case ITEM_TYPE_INCENSE:
            AddStockItem(item, SLATEPORT_SHOP_TM);
            break;
        case ITEM_TYPE_Z_CRYSTAL:
            if (IsFunctionalZCrystal(item))
                AddStockItem(item, SLATEPORT_SHOP_Z);
            break;
        case ITEM_TYPE_TERA_SHARD:
            AddStockItem(item, SLATEPORT_SHOP_TERA);
            break;
        case ITEM_TYPE_GEM:
            AddStockItem(item, SLATEPORT_SHOP_GEMS);
            break;
        case ITEM_TYPE_NATURE_MINT:
            AddStockItem(item, SLATEPORT_SHOP_MINTS);
            break;
        default:
            break;
        }
    }
    AddStockItem(ITEM_MEGA_RING, SLATEPORT_SHOP_ENERGY);
    AddStockItem(ITEM_Z_POWER_RING, SLATEPORT_SHOP_ENERGY);
    AddStockItem(ITEM_TERA_ORB, SLATEPORT_SHOP_ENERGY);
    AddStockItem(ITEM_TM_HIDDEN_POWER, SLATEPORT_SHOP_TM);
    AddStockItem(ITEM_TM_SECRET_POWER, SLATEPORT_SHOP_TM);
    sStockReady = TRUE;
}

static s32 CompareNames(u16 left, u16 right)
{
    u8 leftName[ITEM_NAME_LENGTH + 1], rightName[ITEM_NAME_LENGTH + 1];
    StringCopyUppercase(leftName, GetItemName(left));
    StringCopyUppercase(rightName, GetItemName(right));
    return StringCompare(leftName, rightName);
}

static bool32 IsSpecialStockAvailable(void)
{
    return !FlagGet(FLAG_RUN_RULE_LIMIT_SLATEPORT_SHOP) || FlagGet(FLAG_IS_CHAMPION);
}

void CheckSlateportSpecialStock(void)
{
    gSpecialVar_Result = IsSpecialStockAvailable();
}

u32 BuildSlateportShopStock(u32 category, u16 *items)
{
    u32 count = 0;
    InitStockCategories();
    if (category < SLATEPORT_SHOP_COUNT && (category < SLATEPORT_SHOP_MEGA || IsSpecialStockAvailable()))
    {
        for (u32 item = 1; item < ITEMS_COUNT; item++)
        {
            if (!(sStockCategories[item] & (1 << category)) || !IsItemShopCriteriaFulfilled(item))
                continue;
            u32 index = count++;
            while (index > 0 && CompareNames(items[index - 1], item) > 0)
            {
                items[index] = items[index - 1];
                index--;
            }
            items[index] = item;
        }
    }
    items[count] = ITEM_NONE;
    return count;
}

bool32 IsSlateportPreChampionItem(u16 item)
{
    InitStockCategories();
    return item < ITEMS_COUNT && (sStockCategories[item] & ((1 << SLATEPORT_SHOP_MEGA) - 1)) != 0;
}

bool32 IsFunctionalEvolutionOrFormItem(u16 item)
{
    InitStockCategories();
    return item < ITEMS_COUNT && (sStockCategories[item] & ((1 << SLATEPORT_SHOP_EVOLUTION) | (1 << SLATEPORT_SHOP_FORMS)
        | (1 << SLATEPORT_SHOP_MEGA) | (1 << SLATEPORT_SHOP_Z))) != 0;
}

static bool32 HasFixedCheapPrice(u16 item)
{
    InitStockCategories();
    return item < ITEMS_COUNT && sStockCategories[item] && !(sStockCategories[item] & (1 << SLATEPORT_SHOP_ENERGY))
        && GetItemPocket(item) != POCKET_TM_HM;
}

u32 GetSlateportItemPrice(u16 item, bool32 discount)
{
    if (item == ITEM_MEGA_RING || item == ITEM_Z_POWER_RING || item == ITEM_TERA_ORB)
        return 10000;
    if (HasFixedCheapPrice(item))
        return 100;
    return GetItemPrice(item) >> discount;
}

u32 LimitSlateportResalePrice(u16 item, u32 price)
{
    return HasFixedCheapPrice(item) ? min(price, 50) : price;
}

bool32 TryGiveSlateportPurchase(u32 category, u16 item, u16 count)
{
    InitStockCategories();
    if (category >= SLATEPORT_SHOP_COUNT || item <= ITEM_NONE || item >= ITEMS_COUNT || count == 0
     || !(sStockCategories[item] & (1 << category)) || !IsItemShopCriteriaFulfilled(item)
     || (category >= SLATEPORT_SHOP_MEGA && !IsSpecialStockAvailable())
     || (GetItemImportance(item) && (count != 1 || CheckBagHasItem(item, 1) || CheckPCHasItem(item, 1)))
     || !CheckBagHasSpace(item, count) || !AddBagItem(item, count))
        return FALSE;
    if (item == ITEM_TERA_ORB)
        FlagSet(FLAG_TERA_ORB_CHARGED);
    return TRUE;
}
