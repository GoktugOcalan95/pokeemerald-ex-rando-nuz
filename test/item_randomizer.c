#include "global.h"
#include "event_data.h"
#include "item.h"
#include "item_randomizer.h"
#include "player_pc.h"
#include "pokemon.h"
#include "random.h"
#include "run_randomizer.h"
#include "save.h"
#include "slateport_shops.h"
#include "test/overworld_script.h"
#include "test/test.h"

asm(".set VAR_0x8000, 0x8000\n.set VAR_0x8001, 0x8001\n");

TEST("Item randomizer protects progression and empty held slots in every domain")
{
    const u16 protected[] = {ITEM_NONE, ITEM_HM_SURF, ITEM_TM_FOCUS_PUNCH, ITEM_TOGGLE_REPEL, ITEM_DEVON_GOODS};
    FlagSet(FLAG_RUN_RULE_ITEMS);
    for (u32 domain = ITEM_REWARD_PICKUP; domain <= ITEM_REWARD_FACILITY_HELD; domain++)
        for (u32 i = 0; i < ARRAY_COUNT(protected); i++)
            EXPECT_EQ(RandomizeItemReward(protected[i], domain, 123, 2), protected[i]);
    EXPECT_EQ(RandomizeItemReward(ITEM_RED_ORB, ITEM_REWARD_GIFT, 2, 0), ITEM_RED_ORB);
    EXPECT_EQ(RandomizeItemReward(ITEM_BLUE_ORB, ITEM_REWARD_PICKUP, 2, 0), ITEM_BLUE_ORB);
    FlagClear(FLAG_RUN_RULE_ITEMS);
    for (u32 item = 0; item < ITEMS_COUNT; item++)
        EXPECT_EQ(RandomizeItemReward(item, ITEM_REWARD_PICKUP, 123, 0), item);
}

TEST("Item randomizer excludes unfinished items and retains indirect uses")
{
    const u16 excluded[] = {ITEM_AUX_EVASION, ITEM_AUX_GUARD, ITEM_AUX_POWER, ITEM_AUX_POWERGUARD,
        ITEM_CHOICE_DUMPLING, ITEM_SWAP_SNACK, ITEM_TWICE_SPICED_RADISH};
    const u16 retained[] = {ITEM_LEFTOVERS, ITEM_NUGGET, ITEM_RED_SHARD, ITEM_POKESHI_DOLL};
    FlagSet(FLAG_RUN_RULE_ITEMS);
    FlagClear(FLAG_RUN_RULE_BAN_SLATEPORT);
    FlagClear(FLAG_RUN_RULE_BAN_GIMMICKS);
    FlagClear(FLAG_RUN_RULE_BAN_BATTLE_ITEMS);
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    for (u32 i = 0; i < ARRAY_COUNT(excluded); i++)
        EXPECT(!IsRandomizedRewardItemAllowed(excluded[i]));
    for (u32 i = 0; i < ARRAY_COUNT(retained); i++)
        EXPECT(IsRandomizedRewardItemAllowed(retained[i]));
    FlagClear(FLAG_RUN_RULE_ITEMS);
    for (u32 domain = ITEM_REWARD_PICKUP; domain <= ITEM_REWARD_FACILITY_HELD; domain++)
        for (u32 i = 0; i < ARRAY_COUNT(excluded); i++)
            EXPECT_EQ(RandomizeItemReward(excluded[i], domain, 123, 0), excluded[i]);
}

TEST("Item randomizer shares ability item exclusions and refreshes the No EVs pool")
{
    FlagSet(FLAG_RUN_RULE_ITEMS);
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    EXPECT(IsRandomizedRewardItemAllowed(ITEM_ADAMANT_MINT));
    EXPECT(IsRandomizedRewardItemAllowed(ITEM_HP_UP));
    EXPECT(!IsRandomizedRewardItemAllowed(ITEM_ABILITY_CAPSULE));
    EXPECT(!IsRandomizedRewardItemAllowed(ITEM_ABILITY_PATCH));
    EXPECT(!IsRandomizedRewardItemAllowed(ITEM_WOOD_MAIL));
    for (u32 evs = 0; evs < 2; evs++)
    {
        if (evs)
            FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
        for (u32 domain = ITEM_REWARD_PICKUP; domain <= ITEM_REWARD_FACILITY_HELD; domain++)
            for (u32 source = 0; source < 256; source++)
                EXPECT(IsRandomizedRewardItemAllowed(RandomizeItemReward(ITEM_POTION, domain, source, 0)));
    }
    EXPECT(!IsRandomizedRewardItemAllowed(ITEM_HP_UP));
    EXPECT(IsRandomizedRewardItemAllowed(ITEM_ADAMANT_MINT));
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    FlagClear(FLAG_RUN_RULE_ITEMS);
}

TEST("Item randomizer preserves mappings and gameplay RNG across save load")
{
    u16 expected[8];
    FlagSet(FLAG_RUN_RULE_ITEMS);
    for (u32 i = 0; i < ARRAY_COUNT(expected); i++)
        expected[i] = RandomizeItemReward(ITEM_POTION, ITEM_REWARD_GIFT, 777, i);
    SeedRng(12);
    u16 next = Random();
    SeedRng(12);
    for (u32 i = 0; i < ARRAY_COUNT(expected); i++)
        EXPECT_EQ(RandomizeItemReward(ITEM_ORAN_BERRY, ITEM_REWARD_GIFT, 777, i), expected[i]);
    EXPECT_EQ(Random(), next);
    u32 hash = RunRandomizerHash(1, 2, 3);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    gSaveBlock2Ptr->playerTrainerId[3] ^= 0x80;
    EXPECT_NE(RunRandomizerHash(1, 2, 3), hash);
    FlagClear(FLAG_RUN_RULE_ITEMS);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    for (u32 i = 0; i < ARRAY_COUNT(expected); i++)
        EXPECT_EQ(RandomizeItemReward(ITEM_POTION, ITEM_REWARD_GIFT, 777, i), expected[i]);
    FlagClear(FLAG_RUN_RULE_ITEMS);
}

TEST("Item randomizer gift macro preserves quantity and resolves the same daily slot")
{
    FlagSet(FLAG_RUN_RULE_ITEMS);
    RUN_OVERWORLD_SCRIPT(resolveitem_random ITEM_ORAN_BERRY, 777, 3, 1;);
    u16 item = gSpecialVar_0x8000;
    EXPECT(IsRandomizedRewardItemAllowed(item));
    EXPECT_EQ(gSpecialVar_0x8001, 3);
    RUN_OVERWORLD_SCRIPT(resolveitem_random ITEM_FIGY_BERRY, 777, 3, 1;);
    EXPECT_EQ(gSpecialVar_0x8000, item);
    EXPECT_EQ(gSpecialVar_0x8001, 3);
    FlagClear(FLAG_RUN_RULE_ITEMS);
}

TEST("Item randomizer starting PC uses saved rules and preserves the starting quantity")
{
    FlagClear(FLAG_RUN_RULE_ITEMS);
    NewGameInitPCItems();
    EXPECT_EQ(gSaveBlock1Ptr->pcItems[0].itemId, ITEM_POTION);
    EXPECT_EQ(gSaveBlock1Ptr->pcItems[0].quantity, 1);
    FlagSet(FLAG_RUN_RULE_ITEMS);
    NewGameInitPCItems();
    EXPECT_EQ(gSaveBlock1Ptr->pcItems[0].itemId, RandomizeItemReward(ITEM_POTION, ITEM_REWARD_PC, 0, 0));
    EXPECT_EQ(gSaveBlock1Ptr->pcItems[0].quantity, 1);
    FlagClear(FLAG_RUN_RULE_ITEMS);
}

TEST("Item randomizer preserves wild held probabilities and the RNG stream")
{
    const u16 species[] = {SPECIES_ZIGZAGOON, SPECIES_PIKACHU, SPECIES_CHANSEY, SPECIES_PELIPPER};
    CreateRandomMon(&gParties[B_TRAINER_PLAYER][0], SPECIES_TREECKO, 5);
    for (u32 i = 0; i < ARRAY_COUNT(species); i++)
    {
        CreateRandomMon(&gParties[B_TRAINER_OPPONENT_A][0], species[i], 5);
        for (u32 seed = 0; seed < 100; seed++)
        {
            u16 none = ITEM_NONE;
            FlagClear(FLAG_RUN_RULE_ITEMS);
            SetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_HELD_ITEM, &none);
            SeedRng(seed);
            SetWildMonHeldItem();
            u16 original = GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_HELD_ITEM);
            u16 next = Random();
            FlagSet(FLAG_RUN_RULE_ITEMS);
            SetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_HELD_ITEM, &none);
            SeedRng(seed);
            SetWildMonHeldItem();
            u16 replacement = GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_HELD_ITEM);
            EXPECT_EQ(original == ITEM_NONE, replacement == ITEM_NONE);
            EXPECT_EQ(Random(), next);
            if (replacement != ITEM_NONE)
            {
                EXPECT(IsRandomizedRewardItemAllowed(replacement));
                SetWildMonHeldItem();
                EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_HELD_ITEM), replacement);
            }
        }
    }
    FlagClear(FLAG_RUN_RULE_ITEMS);
}

TEST("Item randomizer scripted gifts retain stable held items and empty slots")
{
    const u8 *script = OVERWORLD_SCRIPT(givemon SPECIES_CASTFORM, 25, item=ITEM_MYSTIC_WATER;);
    FlagSet(FLAG_RUN_RULE_ITEMS);
    ZeroPlayerPartyMons();
    RunScriptImmediately(script);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES), SPECIES_CASTFORM);
    u16 item = GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM);
    EXPECT(IsRandomizedRewardItemAllowed(item));
    ZeroPlayerPartyMons();
    RunScriptImmediately(script);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM), item);
    ZeroPlayerPartyMons();
    RUN_OVERWORLD_SCRIPT(givemon SPECIES_TREECKO, 5, item=ITEM_NONE;);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM), ITEM_NONE);
    FlagClear(FLAG_RUN_RULE_ITEMS);
}

TEST("Item randomizer facility creation preserves authored templates and stable held items")
{
    struct BattleTowerPokemon authored = {
        .species = SPECIES_PIKACHU,
        .level = 50,
        .moves = {MOVE_THUNDER_SHOCK},
        .nickname = _("PIKACHU"),
        .heldItem = ITEM_LIGHT_BALL,
        .personality = 123,
        .otId = 987,
    };
    struct Pokemon mon;
    FlagSet(FLAG_RUN_RULE_ITEMS);
    CreateBattleTowerMon(&mon, &authored);
    u16 item = GetMonData(&mon, MON_DATA_HELD_ITEM);
    EXPECT(IsRandomizedRewardItemAllowed(item));
    CreateBattleTowerMon_HandleLevel(&mon, &authored, 20);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HELD_ITEM), item);
    EXPECT_EQ(authored.heldItem, ITEM_LIGHT_BALL);
    FlagClear(FLAG_RUN_RULE_ITEMS);
    CreateBattleTowerMon(&mon, &authored);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HELD_ITEM), ITEM_LIGHT_BALL);
}

TEST("Ban Slateport items uses pre-Champion stock and leaves shops and mints intact")
{
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    FlagClear(FLAG_RUN_RULE_ITEMS);
    FlagSet(FLAG_RUN_RULE_BAN_SLATEPORT);
    EXPECT(IsRandomizedRewardItemAllowed(ITEM_THUNDER_STONE));
    FlagSet(FLAG_RUN_RULE_ITEMS);
    for (u32 item = 1; item < ITEMS_COUNT; item++)
        if (IsSlateportPreChampionItem(item))
            EXPECT(!IsRandomizedRewardItemAllowed(item));
    EXPECT(IsRandomizedRewardItemAllowed(ITEM_ADAMANT_MINT));
    EXPECT(IsRandomizedRewardItemAllowed(ITEM_VENUSAURITE));
    EXPECT(IsRandomizedRewardItemAllowed(ITEM_FIRE_TERA_SHARD));
    FlagSet(FLAG_IS_CHAMPION);
    EXPECT(!IsRandomizedRewardItemAllowed(ITEM_THUNDER_STONE));
    ClearBag();
    EXPECT(TryGiveSlateportPurchase(SLATEPORT_SHOP_TM, ITEM_THUNDER_STONE, 1));
    for (u32 domain = ITEM_REWARD_PICKUP; domain <= ITEM_REWARD_FACILITY_HELD; domain++)
        for (u32 source = 0; source < 100; source++)
            EXPECT(!IsSlateportPreChampionItem(RandomizeItemReward(ITEM_POTION, domain, source, 0)));
    FlagClear(FLAG_RUN_RULE_BAN_SLATEPORT);
    EXPECT(IsRandomizedRewardItemAllowed(ITEM_THUNDER_STONE));
    FlagClear(FLAG_RUN_RULE_ITEMS);
    FlagClear(FLAG_IS_CHAMPION);
    ClearBag();
}

TEST("Ban gimmick items filters every gimmick category and combines with Slateport")
{
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    FlagClear(FLAG_RUN_RULE_BAN_SLATEPORT);
    FlagSet(FLAG_RUN_RULE_BAN_GIMMICKS);
    FlagClear(FLAG_RUN_RULE_ITEMS);
    EXPECT(IsRandomizedRewardItemAllowed(ITEM_VENUSAURITE));
    FlagSet(FLAG_RUN_RULE_ITEMS);
    u32 counts[3] = {0};
    for (u32 item = 1; item < ITEMS_COUNT; item++)
    {
        u32 type = gItemsInfo[item].sortType;
        if (type >= ITEM_TYPE_MEGA_STONE && type <= ITEM_TYPE_TERA_SHARD)
        {
            counts[type - ITEM_TYPE_MEGA_STONE]++;
            EXPECT(!IsRandomizedRewardItemAllowed(item));
        }
    }
    EXPECT_EQ(counts[0], 92);
    EXPECT_EQ(counts[1], 35);
    EXPECT_EQ(counts[2], 19);
    EXPECT(IsRandomizedRewardItemAllowed(ITEM_ADAMANT_MINT));
    EXPECT(IsRandomizedRewardItemAllowed(ITEM_THUNDER_STONE));
    EXPECT(IsRandomizedRewardItemAllowed(ITEM_ADAMANT_CRYSTAL));
    FlagSet(FLAG_RUN_RULE_BAN_SLATEPORT);
    EXPECT(!IsRandomizedRewardItemAllowed(ITEM_THUNDER_STONE));
    EXPECT(IsRandomizedRewardItemAllowed(ITEM_ADAMANT_MINT));
    for (u32 domain = ITEM_REWARD_PICKUP; domain <= ITEM_REWARD_FACILITY_HELD; domain++)
        for (u32 source = 0; source < 100; source++)
            EXPECT(IsRandomizedRewardItemAllowed(RandomizeItemReward(ITEM_POTION, domain, source, 0)));
    FlagClear(FLAG_RUN_RULE_BAN_GIMMICKS);
    FlagClear(FLAG_RUN_RULE_BAN_SLATEPORT);
    EXPECT(IsRandomizedRewardItemAllowed(ITEM_VENUSAURITE));
    FlagClear(FLAG_RUN_RULE_ITEMS);
}

TEST("Ban in-battle items excludes exact unwanted rewards and retains useful supplies")
{
    const u16 banned[] = {ITEM_X_ATTACK, ITEM_X_DEFENSE, ITEM_X_SP_ATK, ITEM_X_SP_DEF, ITEM_X_SPEED, ITEM_X_ACCURACY,
        ITEM_DIRE_HIT, ITEM_GUARD_SPEC, ITEM_BLUE_FLUTE, ITEM_YELLOW_FLUTE, ITEM_RED_FLUTE, ITEM_BLACK_FLUTE,
        ITEM_WHITE_FLUTE, ITEM_POKE_DOLL, ITEM_FLUFFY_TAIL, ITEM_POKE_TOY, ITEM_MAX_MUSHROOMS};
    const u16 retained[] = {ITEM_POTION, ITEM_ANTIDOTE, ITEM_ETHER, ITEM_POKE_BALL, ITEM_ADAMANT_MINT};
    FlagClear(FLAG_RUN_RULE_ITEMS);
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    FlagSet(FLAG_RUN_RULE_BAN_BATTLE_ITEMS);
    for (u32 i = 0; i < ARRAY_COUNT(banned); i++)
        EXPECT(IsRandomizedRewardItemAllowed(banned[i]));
    FlagSet(FLAG_RUN_RULE_ITEMS);
    for (u32 item = 1; item < ITEMS_COUNT; item++)
    {
        FlagClear(FLAG_RUN_RULE_BAN_BATTLE_ITEMS);
        bool32 original = IsRandomizedRewardItemAllowed(item);
        FlagSet(FLAG_RUN_RULE_BAN_BATTLE_ITEMS);
        bool32 expected = original;
        for (u32 i = 0; i < ARRAY_COUNT(banned); i++)
            if (item == banned[i])
                expected = FALSE;
        EXPECT_EQ(IsRandomizedRewardItemAllowed(item), expected);
    }
    for (u32 i = 0; i < ARRAY_COUNT(retained); i++)
        EXPECT(IsRandomizedRewardItemAllowed(retained[i]));
    FlagSet(FLAG_RUN_RULE_BAN_SLATEPORT);
    FlagSet(FLAG_RUN_RULE_BAN_GIMMICKS);
    FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    for (u32 domain = ITEM_REWARD_PICKUP; domain <= ITEM_REWARD_FACILITY_HELD; domain++)
        for (u32 source = 0; source < 256; source++)
            EXPECT(IsRandomizedRewardItemAllowed(RandomizeItemReward(ITEM_POTION, domain, source, 0)));
    EXPECT(IsRandomizedRewardItemAllowed(ITEM_ADAMANT_MINT));
    FlagClear(FLAG_RUN_RULE_BAN_SLATEPORT);
    FlagClear(FLAG_RUN_RULE_BAN_GIMMICKS);
    FlagClear(FLAG_RUN_RULE_BAN_BATTLE_ITEMS);
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    FlagClear(FLAG_RUN_RULE_ITEMS);
}
