#include "global.h"
#include "boss_rewards.h"
#include "event_data.h"
#include "item.h"
#include "save.h"
#include "test/test.h"
#include "constants/opponents.h"
#include "constants/flags.h"
#include "constants/vars.h"

static void ResetRewards(void)
{
    ClearBag();
    VarSet(VAR_BOSS_REWARDS_EARNED, 0);
    VarSet(VAR_BOSS_REWARDS_DELIVERED, 0);
}

TEST("Boss rewards retain fixed assignments in either progression order")
{
    static const u16 trainers[] = {
        TRAINER_ROXANNE_1, TRAINER_MAY_RUSTBORO_TREECKO, TRAINER_BRAWLY_1,
        TRAINER_MAY_ROUTE_110_TREECKO, TRAINER_WATTSON_1, TRAINER_FLANNERY_1,
        TRAINER_NORMAN_1, TRAINER_MAY_ROUTE_119_TREECKO, TRAINER_WINONA_1,
        TRAINER_MAY_LILYCOVE_TREECKO, TRAINER_TATE_AND_LIZA_1, TRAINER_JUAN_1,
        TRAINER_WALLY_VR_1,
    };
    bool32 reverse;
    PARAMETRIZE { reverse = FALSE; }
    PARAMETRIZE { reverse = TRUE; }

    ResetRewards();
    FlagSet(FLAG_RUN_RULE_ITEMS);
    for (u32 i = 0; i < ARRAY_COUNT(trainers); i++)
    {
        u32 index = reverse ? ARRAY_COUNT(trainers) - 1 - i : i;
        BossRewards_RecordVictory(trainers[index]);
        EXPECT_EQ(BossRewards_TryDeliver(), index % 2 ? ITEM_ABILITY_PATCH : ITEM_ABILITY_CAPSULE);
        BossRewards_RecordVictory(trainers[index]);
        EXPECT_EQ(BossRewards_TryDeliver(), ITEM_NONE);
    }
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_ABILITY_CAPSULE), 7);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_ABILITY_PATCH), 6);
    ResetRewards();
}

TEST("Boss rewards share rival branches and exclude opening rivals and rematches")
{
    static const u16 rivals[] = {
        TRAINER_MAY_RUSTBORO_TREECKO, TRAINER_MAY_RUSTBORO_TORCHIC, TRAINER_MAY_RUSTBORO_MUDKIP,
        TRAINER_BRENDAN_RUSTBORO_TREECKO, TRAINER_BRENDAN_RUSTBORO_TORCHIC, TRAINER_BRENDAN_RUSTBORO_MUDKIP,
    };
    ResetRewards();
    for (u32 i = 0; i < ARRAY_COUNT(rivals); i++)
        BossRewards_RecordVictory(rivals[i]);
    EXPECT_EQ(BossRewards_TryDeliver(), ITEM_ABILITY_PATCH);
    EXPECT_EQ(BossRewards_TryDeliver(), ITEM_NONE);
    BossRewards_RecordVictory(TRAINER_ROXANNE_2);
    BossRewards_RecordVictory(TRAINER_WALLY_VR_2);
    BossRewards_RecordVictory(TRAINER_MAY_ROUTE_103_TREECKO);
    EXPECT_EQ(BossRewards_TryDeliver(), ITEM_NONE);
    BossRewards_RecordVictory(TRAINER_JUAN_1);
    EXPECT_EQ(BossRewards_TryDeliver(), ITEM_ABILITY_PATCH);
    ResetRewards();
}

TEST("Boss rewards preserve pending and delivered awards across save load and a full Bag")
{
    ResetRewards();
    while (AddBagItem(ITEM_ABILITY_CAPSULE, MAX_BAG_ITEM_CAPACITY))
        ;
    BossRewards_RecordVictory(TRAINER_ROXANNE_1);
    EXPECT_EQ(BossRewards_TryDeliver(), ITEM_NONE);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    ResetRewards();
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(BossRewards_TryDeliver(), ITEM_NONE);
    EXPECT(RemoveBagItem(ITEM_ABILITY_CAPSULE, 1));
    EXPECT_EQ(BossRewards_TryDeliver(), ITEM_ABILITY_CAPSULE);
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    ResetRewards();
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    ClearBag();
    BossRewards_RecordVictory(TRAINER_ROXANNE_1);
    EXPECT_EQ(BossRewards_TryDeliver(), ITEM_NONE);
    ResetRewards();
}

TEST("Boss rewards reserve only ability customization items")
{
    EXPECT(IsAbilityCustomizationItem(ITEM_ABILITY_CAPSULE));
    EXPECT(IsAbilityCustomizationItem(ITEM_ABILITY_PATCH));
    EXPECT(!IsAbilityCustomizationItem(ITEM_ADAMANT_MINT));
    EXPECT(!IsAbilityCustomizationItem(ITEM_NONE));
}
