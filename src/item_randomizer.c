#include "global.h"
#include "item_randomizer.h"
#include "boss_rewards.h"
#include "event_data.h"
#include "item.h"
#include "mail.h"
#include "no_evs.h"
#include "run_randomizer.h"
#include "script.h"
#include "slateport_shops.h"
#include "battle_pyramid.h"
#include "teaching_randomizer.h"

static EWRAM_DATA u16 sRewardPool[ITEMS_COUNT] = {0};
static EWRAM_DATA u16 sRewardPoolCount = 0;
static EWRAM_DATA u32 sRewardPoolRules = 0;

static bool32 IsUnimplementedItem(u16 item)
{
    switch (item)
    {
    case ITEM_AUX_EVASION:
    case ITEM_AUX_GUARD:
    case ITEM_AUX_POWER:
    case ITEM_AUX_POWERGUARD:
    case ITEM_CHOICE_DUMPLING:
    case ITEM_SWAP_SNACK:
    case ITEM_TWICE_SPICED_RADISH:
        return TRUE;
    default:
        return FALSE;
    }
}

static bool32 IsBannedGimmickItem(u16 item)
{
    enum ItemSortType type = gItemsInfo[item].sortType;
    return (type == ITEM_TYPE_MEGA_STONE && FlagGet(FLAG_RUN_RULE_BAN_MEGA_STONES))
        || (type == ITEM_TYPE_Z_CRYSTAL && FlagGet(FLAG_RUN_RULE_BAN_Z_CRYSTALS))
        || (type == ITEM_TYPE_TERA_SHARD && FlagGet(FLAG_RUN_RULE_BAN_TERA_SHARDS))
        || (type == ITEM_TYPE_GEM && FlagGet(FLAG_RUN_RULE_BAN_TYPE_GEMS));
}

static bool32 IsBannedBattleItem(u16 item)
{
    return gItemsInfo[item].sortType == ITEM_TYPE_X_ITEM
        || item == ITEM_BLUE_FLUTE || item == ITEM_YELLOW_FLUTE || item == ITEM_RED_FLUTE
        || item == ITEM_BLACK_FLUTE || item == ITEM_WHITE_FLUTE
        || item == ITEM_POKE_DOLL || item == ITEM_FLUFFY_TAIL || item == ITEM_POKE_TOY || item == ITEM_MAX_MUSHROOMS;
}

bool32 IsRandomizedRewardItemAllowed(u16 item)
{
    return item > ITEM_NONE && item < ITEMS_COUNT && gItemsInfo[item].name != NULL
        && GetItemPocket(item) != POCKET_KEY_ITEMS && GetItemPocket(item) != POCKET_TM_HM
        && !(FlagGet(FLAG_RUN_RULE_ITEMS) && FlagGet(FLAG_RUN_RULE_BAN_SLATEPORT) && IsSlateportPreChampionItem(item))
        && !(FlagGet(FLAG_RUN_RULE_ITEMS) && IsBannedGimmickItem(item))
        && !(FlagGet(FLAG_RUN_RULE_ITEMS) && FlagGet(FLAG_RUN_RULE_BAN_BATTLE_ITEMS) && IsBannedBattleItem(item))
        && !IsUnimplementedItem(item) && !ItemIsMail(item) && !IsAbilityCustomizationItem(item) && IsItemAllowedByNoEVs(item);
}

bool32 IsRandomizedLootItemAllowed(u16 item)
{
    return IsRandomizedRewardItemAllowed(item)
        || (FlagGet(FLAG_RUN_RULE_ITEMS) && !GetItemImportance(ITEM_TM01)
         && item >= ITEM_TM01 && item < ITEM_TM01 + GetActiveTMCount());
}

static void PrepareRewardPool(bool32 heldItems)
{
    u32 rules = FlagGet(FLAG_RUN_RULE_NO_EV_GAIN) | (FlagGet(FLAG_RUN_RULE_BAN_SLATEPORT) << 1)
        | (FlagGet(FLAG_RUN_RULE_BAN_MEGA_STONES) << 2) | (FlagGet(FLAG_RUN_RULE_BAN_BATTLE_ITEMS) << 3)
        | (FlagGet(FLAG_RUN_RULE_BAN_Z_CRYSTALS) << 4) | (FlagGet(FLAG_RUN_RULE_BAN_TERA_SHARDS) << 5)
        | (FlagGet(FLAG_RUN_RULE_BAN_TYPE_GEMS) << 6)
        | (FlagGet(FLAG_RUN_RULE_REUSABLE_TMS) << 7) | (IsExpandedTMListEnabled() << 8) | (heldItems << 9);
    if (sRewardPoolCount != 0 && rules == sRewardPoolRules)
        return;
    sRewardPoolCount = 0;
    for (u32 item = 1; item < ITEMS_COUNT; item++)
        if (heldItems ? IsRandomizedRewardItemAllowed(item) : IsRandomizedLootItemAllowed(item))
            sRewardPool[sRewardPoolCount++] = item;
    sRewardPoolRules = rules;
}

u16 RandomizeItemReward(u16 original, u32 domain, u32 source, u32 slot)
{
    if (!FlagGet(FLAG_RUN_RULE_ITEMS) || original == ITEM_NONE || original >= ITEMS_COUNT
     || GetItemPocket(original) == POCKET_KEY_ITEMS || GetItemPocket(original) == POCKET_TM_HM)
        return original;
    if (domain < ITEM_REWARD_WILD_HELD && (original == ITEM_RED_ORB || original == ITEM_BLUE_ORB))
        return original;
    PrepareRewardPool(domain >= ITEM_REWARD_WILD_HELD && domain <= ITEM_REWARD_FACILITY_HELD);
    if (sRewardPoolCount == 0)
        return original;
    return sRewardPool[RunRandomizerHash(domain, source, slot) % sRewardPoolCount];
}

void RandomizePickupFromScript(void)
{
    u32 source = (gSaveBlock1Ptr->location.mapGroup << 16) | (gSaveBlock1Ptr->location.mapNum << 8) | gSpecialVar_LastTalked;
    if (CurrentBattlePyramidLocation() == PYRAMID_LOCATION_NONE)
        gSpecialVar_0x8000 = RandomizeItemReward(gSpecialVar_0x8000, ITEM_REWARD_PICKUP, source, 0);
}

void RandomizeHiddenItemFromScript(void)
{
    gSpecialVar_0x8005 = RandomizeItemReward(gSpecialVar_0x8005, ITEM_REWARD_HIDDEN, gSpecialVar_0x8004, 0);
}

void RandomizeGiftFromScript(struct ScriptContext *ctx)
{
    u32 source = ScriptReadHalfword(ctx);
    u32 slot = ScriptReadByte(ctx);
    gSpecialVar_0x8000 = RandomizeItemReward(gSpecialVar_0x8000, ITEM_REWARD_GIFT, source, slot);
}

void RandomizeFreeGiftFromScript(struct ScriptContext *ctx)
{
    gSpecialVar_0x8000 = RandomizeItemReward(gSpecialVar_0x8000, ITEM_REWARD_GIFT, (u32)ctx->scriptPtr, 0);
}

bool32 AddAuthoredItemReward(u16 original, u16 item, u16 count)
{
    u16 partner = IsExpandedTMListEnabled() && original >= ITEM_TM01 && original <= ITEM_TM50 ? original + 50 : ITEM_NONE;
    if (partner != ITEM_NONE && !CheckBagHasSpace(partner, count))
        return FALSE;
    if (!AddBagItem(item, count))
        return FALSE;
    if (partner != ITEM_NONE && !AddBagItem(partner, count))
    {
        RemoveBagItem(item, count);
        return FALSE;
    }
    return TRUE;
}

void GiveAuthoredItemFromScript(struct ScriptContext *ctx)
{
    u16 original = gSpecialVar_0x8000;
    RandomizeFreeGiftFromScript(ctx);
    gSpecialVar_Result = AddAuthoredItemReward(original, gSpecialVar_0x8000, gSpecialVar_0x8001);
    gSpecialVar_0x800B = gSpecialVar_Result && IsExpandedTMListEnabled() && original >= ITEM_TM01 && original <= ITEM_TM50 ? original + 50 : ITEM_NONE;
}

void GiveAuthoredPickupFromScript(void)
{
    u16 original = gSpecialVar_0x8000;
    RandomizePickupFromScript();
    gSpecialVar_Result = AddAuthoredItemReward(original, gSpecialVar_0x8000, gSpecialVar_0x8001);
    gSpecialVar_0x800B = gSpecialVar_Result && IsExpandedTMListEnabled() && original >= ITEM_TM01 && original <= ITEM_TM50 ? original + 50 : ITEM_NONE;
}
