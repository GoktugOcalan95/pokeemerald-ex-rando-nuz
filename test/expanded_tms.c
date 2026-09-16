#include "global.h"
#include "coins.h"
#include "event_data.h"
#include "expanded_tms.h"
#include "item.h"
#include "item_randomizer.h"
#include "run_randomizer.h"
#include "run_setup.h"
#include "save.h"
#include "shop_criteria.h"
#include "slateport_shops.h"
#include "teaching_randomizer.h"
#include "test/test.h"
#include "constants/flags.h"

TEST("Expanded TMs require randomized teaching and retain their saved preset choice")
{
    RunSetup_Begin();
    RunSetup_SetPreset(RUN_SETUP_PRESET_BISHEY);
    EXPECT(RunSetup_IsAvailable(RUN_SETUP_EXPANDED_TMS));
    RunSetup_SetValue(RUN_SETUP_TMS_TUTORS, FALSE);
    EXPECT(!RunSetup_IsAvailable(RUN_SETUP_EXPANDED_TMS));
    EXPECT(RunSetup_GetValue(RUN_SETUP_EXPANDED_TMS));
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();
    EXPECT(!IsExpandedTMListEnabled());
    EXPECT_EQ(GetItemTMHMMoveId(ITEM_TM51), MOVE_NONE);
    RunSetup_Begin();
    RunSetup_SetPreset(RUN_SETUP_PRESET_BISHEY);
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();
    u16 move = GetItemTMHMMoveId(ITEM_TM100);
    EXPECT_NE(move, MOVE_NONE);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    FlagClear(FLAG_RUN_RULE_EXPANDED_TMS);
    EXPECT_EQ(GetItemTMHMMoveId(ITEM_TM100), MOVE_NONE);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(GetItemTMHMMoveId(ITEM_TM100), move);
}

TEST("Expanded TMs pair only authored rewards and roll back when the complete pair cannot fit")
{
    FlagSet(FLAG_RUN_RULE_TMS_TUTORS);
    FlagSet(FLAG_RUN_RULE_EXPANDED_TMS);
    ClearBag();
    EXPECT(AddAuthoredItemReward(ITEM_TM01, ITEM_TM01, 5));
    EXPECT(CheckBagHasItem(ITEM_TM01, 5));
    EXPECT(CheckBagHasItem(ITEM_TM51, 5));
    EXPECT(AddAuthoredItemReward(ITEM_POKE_BALL, ITEM_TM30, 5));
    EXPECT(CheckBagHasItem(ITEM_TM30, 5));
    EXPECT(!CheckBagHasItem(ITEM_TM80, 1));
    ClearBag();
    u16 capacity = gBagPockets[BAG_TM_HM].capacity;
    gBagPockets[BAG_TM_HM].capacity = 1;
    EXPECT(!AddAuthoredItemReward(ITEM_TM50, ITEM_TM50, 1));
    EXPECT(!CheckBagHasItem(ITEM_TM50, 1));
    EXPECT(!CheckBagHasItem(ITEM_TM100, 1));
    gBagPockets[BAG_TM_HM].capacity = capacity;
    EXPECT(AddAuthoredItemReward(ITEM_TM50, ITEM_TM50, 1));
    EXPECT(CheckBagHasItem(ITEM_TM100, 1));
    FlagClear(FLAG_RUN_RULE_EXPANDED_TMS);
    EXPECT(AddAuthoredItemReward(ITEM_TM09, ITEM_TM09, 1));
    EXPECT(!CheckBagHasItem(ITEM_TM59, 1));
}

TEST("Expanded TMs add separate shop stock at matching prices and Game Corner charges fifty coins")
{
    const u16 original[] = {ITEM_TM13, ITEM_TM50, ITEM_NONE};
    const u16 *stock = original;
    u16 count = 2;
    FlagSet(FLAG_RUN_RULE_TMS_TUTORS);
    FlagSet(FLAG_RUN_RULE_EXPANDED_TMS);
    TryBuildDynamicShopItemList(&stock, &count);
    EXPECT_EQ(count, 4);
    EXPECT_EQ(stock[0], ITEM_TM13);
    EXPECT_EQ(stock[1], ITEM_TM63);
    EXPECT_EQ(stock[2], ITEM_TM50);
    EXPECT_EQ(stock[3], ITEM_TM100);
    EXPECT_EQ(GetItemPrice(ITEM_TM63), GetItemPrice(ITEM_TM13));
    EXPECT_EQ(GetItemPrice(ITEM_TM100), GetItemPrice(ITEM_TM50));
    count = TryFreeDynamicShopItemList(&stock);
    EXPECT_EQ(count, 2);
    TryBuildDynamicShopItemList(&stock, &count);
    EXPECT_EQ(count, 4);
    TryFreeDynamicShopItemList(&stock);
    ClearBag();
    EXPECT(TryGiveSlateportPurchase(SLATEPORT_SHOP_TM, ITEM_TM60, 1));
    EXPECT(TryGiveSlateportPurchase(SLATEPORT_SHOP_TM, ITEM_TM93, 1));
    EXPECT(!TryGiveSlateportPurchase(SLATEPORT_SHOP_TM, ITEM_TM82, 1));
    SetCoins(49);
    EXPECT_EQ(BuyGameCornerTM(ITEM_TM13), 2);
    EXPECT_EQ(GetCoins(), 49);
    SetCoins(50);
    u16 capacity = gBagPockets[BAG_TM_HM].capacity;
    gBagPockets[BAG_TM_HM].capacity = 2;
    EXPECT_EQ(BuyGameCornerTM(ITEM_TM13), 0);
    EXPECT_EQ(GetCoins(), 50);
    gBagPockets[BAG_TM_HM].capacity = capacity;
    EXPECT_EQ(BuyGameCornerTM(ITEM_TM13), 1);
    EXPECT_EQ(GetCoins(), 0);
    EXPECT(!CheckBagHasItem(ITEM_TM63, 1));
    ClearBag();
    SetCoins(500);
    for (u32 extra = 0; extra < 2; extra++)
        for (u32 i = 0; i < 5; i++)
            EXPECT_EQ(BuyGameCornerTM(GetGameCornerTM(i, extra)), 1);
    EXPECT_EQ(GetCoins(), 0);
    FlagSet(FLAG_RUN_RULE_REUSABLE_TMS);
    SetCoins(50);
    EXPECT_EQ(BuyGameCornerTM(ITEM_TM13), 3);
    EXPECT_EQ(GetCoins(), 50);
}

TEST("Expanded TMs join consumable loot with equal item weights and never enter held pools")
{
    bool32 expanded = FALSE, reusable = FALSE;
    PARAMETRIZE { expanded = FALSE; reusable = FALSE; }
    PARAMETRIZE { expanded = TRUE; reusable = FALSE; }
    PARAMETRIZE { expanded = FALSE; reusable = TRUE; }
    PARAMETRIZE { expanded = TRUE; reusable = TRUE; }
    FlagSet(FLAG_RUN_RULE_ITEMS);
    FlagSet(FLAG_RUN_RULE_TMS_TUTORS);
    if (expanded) FlagSet(FLAG_RUN_RULE_EXPANDED_TMS);
    if (reusable) FlagSet(FLAG_RUN_RULE_REUSABLE_TMS);
    u16 pool[ITEMS_COUNT];
    u32 count = 0;
    for (u32 i = 1; i < ITEMS_COUNT; i++)
        if (IsRandomizedRewardItemAllowed(i) || (!reusable && i >= ITEM_TM01 && i < ITEM_TM01 + (expanded ? 100 : 50)))
            pool[count++] = i;
    bool32 foundTM = FALSE;
    for (u32 source = 0; source < 2000; source++)
    {
        u16 item = RandomizeItemReward(ITEM_POTION, ITEM_REWARD_GIFT, source, 0);
        EXPECT_EQ(item, pool[RunRandomizerHash(ITEM_REWARD_GIFT, source, 0) % count]);
        foundTM |= item >= ITEM_TM01 && item <= ITEM_TM100;

    }
    for (u32 source = 0; source < 256; source++)
        EXPECT_NE(GetItemPocket(RandomizeItemReward(ITEM_POTION, ITEM_REWARD_WILD_HELD, source, 0)), POCKET_TM_HM);
    EXPECT_EQ(foundTM, !reusable);
}
