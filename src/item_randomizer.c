#include "global.h"
#include "item_randomizer.h"
#include "boss_rewards.h"
#include "event_data.h"
#include "item.h"
#include "mail.h"
#include "no_evs.h"
#include "run_randomizer.h"
#include "script.h"
#include "battle_pyramid.h"

static EWRAM_DATA u16 sRewardPool[ITEMS_COUNT] = {0};
static EWRAM_DATA u16 sRewardPoolCount = 0;
static EWRAM_DATA u32 sRewardPoolRules = 0;

bool32 IsRandomizedRewardItemAllowed(u16 item)
{
    return item > ITEM_NONE && item < ITEMS_COUNT && gItemsInfo[item].name != NULL
        && GetItemPocket(item) != POCKET_KEY_ITEMS && GetItemPocket(item) != POCKET_TM_HM
        && !ItemIsMail(item) && !IsAbilityCustomizationItem(item) && IsItemAllowedByNoEVs(item);
}

static void PrepareRewardPool(void)
{
    u32 rules = FlagGet(FLAG_RUN_RULE_NO_EV_GAIN);
    if (sRewardPoolCount != 0 && rules == sRewardPoolRules)
        return;
    sRewardPoolCount = 0;
    for (u32 item = 1; item < ITEMS_COUNT; item++)
        if (IsRandomizedRewardItemAllowed(item))
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
    PrepareRewardPool();
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
