#include "global.h"
#include "no_evs.h"
#include "item.h"
#include "pokemon_storage_system.h"
#include "shop_criteria.h"
#include "list_menu.h"
#include "constants/field_specials.h"
#include "event_data.h"
#include "pokemon.h"
#include "run_setup.h"
#include "save.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/items.h"

u32 Test_BuildScrollableMultichoiceItems(u32 menu, u32 count, struct ListMenuItem *items);

TEST("No EVs filters BP and Berry Powder menus while preserving purchase and Exit IDs")
{
    struct ListMenuItem items[12];
    const u8 powderIds[] = {0, 1, 2, 3, 10, 11};
    FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    EXPECT_EQ(Test_BuildScrollableMultichoiceItems(SCROLL_MULTI_BERRY_POWDER_VENDOR, 12, items), 6);
    for (u32 i = 0; i < ARRAY_COUNT(powderIds); i++)
    {
        EXPECT_EQ(items[i].id, powderIds[i]);
        EXPECT_NE(items[i].name, NULL);
    }
    EXPECT_EQ(Test_BuildScrollableMultichoiceItems(SCROLL_MULTI_BF_EXCHANGE_CORNER_VITAMIN_VENDOR, 7, items), 1);
    EXPECT_EQ(items[0].id, 6);
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    EXPECT_EQ(Test_BuildScrollableMultichoiceItems(SCROLL_MULTI_BERRY_POWDER_VENDOR, 12, items), 12);
    for (u32 i = 0; i < 12; i++)
        EXPECT_EQ(items[i].id, i);
    EXPECT_EQ(Test_BuildScrollableMultichoiceItems(SCROLL_MULTI_BF_EXCHANGE_CORNER_VITAMIN_VENDOR, 7, items), 7);
}

TEST("No EVs overrides authored facility EV spreads on both sides")
{
    struct BattleTowerPokemon authored = {
        .species = SPECIES_MAGIKARP,
        .level = 50,
        .moves = {MOVE_SPLASH},
        .nickname = _("MAGIKARP"),
        .hpEV = 84,
        .attackEV = 84,
        .defenseEV = 84,
        .speedEV = 84,
        .spAttackEV = 84,
        .spDefenseEV = 84,
    };
    bool32 enabled = FALSE;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    if (enabled)
        FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    else
        FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    for (u32 trainer = 0; trainer < ARRAY_COUNT(gParties); trainer++)
    {
        struct Pokemon *mon = &gParties[trainer][0];
        CreateBattleTowerMon(mon, &authored);
        EXPECT_EQ(GetMonEVCount(mon), enabled ? 0 : 504);
        EXPECT_EQ(mon->maxHP, enabled ? 80 : 90);
    }
    EXPECT_EQ(authored.hpEV, 84);
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
}

TEST("No EVs normalizes imported copies and cached stats without changing other individual data")
{
    struct Pokemon authored, copy, baseline;
    struct BoxPokemon box;
    u8 ev = 84, cool = 100, unlocked = TRUE;
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    CreateMonWithIVs(&baseline, SPECIES_MAGIKARP, 50, 12345, OTID_STRUCT_PRESET(54321), 31);
    authored = baseline;
    for (u32 stat = 0; stat < NUM_STATS; stat++)
        SetMonData(&authored, MON_DATA_HP_EV + stat, &ev);
    SetMonData(&authored, MON_DATA_COOL, &cool);
    SetMonData(&authored, MON_DATA_TERA_UNLOCKED, &unlocked);
    CalculateMonStats(&authored);
    EXPECT_GT(authored.maxHP, baseline.maxHP);
    FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    CopyMon(&copy, &authored, sizeof(copy));
    EXPECT_EQ(GetMonEVCount(&copy), 0);
    EXPECT_EQ(copy.maxHP, baseline.maxHP);
    EXPECT_EQ(copy.attack, baseline.attack);
    EXPECT_EQ(copy.speed, baseline.speed);
    EXPECT_EQ(GetMonData(&copy, MON_DATA_COOL), cool);
    EXPECT_EQ(GetMonData(&copy, MON_DATA_TERA_UNLOCKED), TRUE);
    EXPECT_EQ(GetMonData(&copy, MON_DATA_PERSONALITY), 12345);
    EXPECT_EQ(GetMonData(&copy, MON_DATA_SANITY_IS_BAD_EGG), FALSE);
    CopyMon(&box, &authored.box, sizeof(box));
    for (u32 stat = 0; stat < NUM_STATS; stat++)
        EXPECT_EQ(GetBoxMonData(&box, MON_DATA_HP_EV + stat), 0);
    EXPECT_EQ(GetMonEVCount(&authored), ev * NUM_STATS);
    authored.hp = 0;
    NormalizeMonEVs(&authored);
    EXPECT_EQ(authored.hp, 0);
    EXPECT_EQ(authored.maxHP, baseline.maxHP);
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    EXPECT_EQ(GetMonEVCount(&authored), 0);
}

TEST("No EVs filters exactly the approved training items from shop stock independently of randomization")
{
    const u16 original[] = {ITEM_HP_UP, ITEM_POWER_WEIGHT, ITEM_GLIMMERING_CHARM, ITEM_FRESH_START_MOCHI,
        ITEM_PRETTY_FEATHER, ITEM_POMEG_BERRY, ITEM_ADAMANT_MINT, ITEM_POTION, ITEM_NONE};
    const u16 *stock = original;
    u16 count = ARRAY_COUNT(original) - 1;
    u32 blocked = 0;
    FlagClear(FLAG_RUN_RULE_ITEMS);
    FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    for (u32 item = 1; item < ITEMS_COUNT; item++)
    {
        if (IsEVRelatedItem(item))
        {
            blocked++;
            EXPECT(!IsItemAllowedByNoEVs(item));
            EXPECT(!IsItemShopCriteriaFulfilled(item));
        }
    }
    EXPECT_EQ(blocked, 27);
    TryBuildDynamicShopItemList(&stock, &count);
    EXPECT_EQ(count, 4);
    for (u32 i = 0; i < count; i++)
        EXPECT_EQ(stock[i], original[i + 4]);
    TryFreeDynamicShopItemList(&stock);
    EXPECT_EQ(stock, original);
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    for (u32 item = 1; item < ITEMS_COUNT; item++)
        EXPECT(IsItemAllowedByNoEVs(item));
}

TEST("No EVs keeps party PC fusion and daycare EVs zero after save load")
{
    struct Pokemon mon;
    u8 ev = 84;
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    CreateRandomMonWithIVs(&mon, SPECIES_WOBBUFFET, 50, 0);
    for (u32 stat = 0; stat < NUM_STATS; stat++)
        SetMonData(&mon, MON_DATA_HP_EV + stat, &ev);
    CalculateMonStats(&mon);
    gParties[B_TRAINER_PLAYER][0] = mon;
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    gPokemonStoragePtr->boxes[0][0] = mon.box;
    gPokemonStoragePtr->fusions[0] = mon;
    gSaveBlock1Ptr->daycare.mons[0].mon = mon.box;
    FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    NormalizeStoredMonEVs();
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(GetMonEVCount(&gParties[B_TRAINER_PLAYER][0]), 0);
    EXPECT_EQ(GetMonEVCount(&gSaveBlock1Ptr->playerParty[0]), 0);
    EXPECT_EQ(GetMonEVCount(&gPokemonStoragePtr->fusions[0]), 0);
    EXPECT_LT(gParties[B_TRAINER_PLAYER][0].maxHP, mon.maxHP);
    for (u32 stat = 0; stat < NUM_STATS; stat++)
    {
        EXPECT_EQ(GetBoxMonData(&gPokemonStoragePtr->boxes[0][0], MON_DATA_HP_EV + stat), 0);
        EXPECT_EQ(GetBoxMonData(&gSaveBlock1Ptr->daycare.mons[0].mon, MON_DATA_HP_EV + stat), 0);
    }
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
}

TEST("No EVs rejects authored EVs and blocks battle training bonuses")
{
    struct Pokemon mon;
    bool32 enabled = FALSE;
    enum Item item = ITEM_NONE;
    u8 pokerus = 0x11;
    u8 ev = 10;

    for (u32 rule = FALSE; rule <= TRUE; rule++)
    {
        PARAMETRIZE { enabled = rule; item = ITEM_NONE; }
        PARAMETRIZE { enabled = rule; item = ITEM_MACHO_BRACE; }
        PARAMETRIZE { enabled = rule; item = ITEM_POWER_BRACER; }
    }

    if (enabled)
        FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    else
        FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    CreateRandomMonWithIVs(&mon, SPECIES_WOBBUFFET, 50, 0);
    SetMonData(&mon, MON_DATA_HELD_ITEM, &item);
    SetMonData(&mon, MON_DATA_POKERUS, &pokerus);
    for (u32 stat = 0; stat < NUM_STATS; stat++)
        SetMonData(&mon, MON_DATA_HP_EV + stat, &ev);
    MonGainEVs(&mon, SPECIES_CATERPIE);
    if (enabled)
    {
        for (u32 stat = 0; stat < NUM_STATS; stat++)
            EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_EV + stat), 0);
    }
    else
    {
        EXPECT_GT(GetMonData(&mon, MON_DATA_HP_EV), ev);
        if (item == ITEM_POWER_BRACER)
            EXPECT_GT(GetMonData(&mon, MON_DATA_ATK_EV), ev);
    }
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
}

TEST("No EVs run rule survives saving and loading")
{
    bool32 enabled = FALSE;
    struct Pokemon mon;

    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }

    RunSetup_Begin();
    RunSetup_SetNoEVGain(enabled);
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    if (enabled)
        FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    else
        FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_NO_EV_GAIN), enabled);
    CreateRandomMonWithIVs(&mon, SPECIES_WOBBUFFET, 50, 0);
    MonGainEVs(&mon, SPECIES_CATERPIE);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_EV), enabled ? 0 : gSpeciesInfo[SPECIES_CATERPIE].evYield_HP);
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
}

TEST("No EVs rejects all EV-raising items without changing EVs or friendship")
{
    struct Pokemon mon;
    bool32 enabled = FALSE;
    enum Item item = ITEM_NONE;
    u8 ev = 20;
    u8 friendship = 50;

    for (u32 rule = FALSE; rule <= TRUE; rule++)
    {
        for (enum Item vitamin = ITEM_HP_UP; vitamin <= ITEM_CARBOS; vitamin++)
            PARAMETRIZE { enabled = rule; item = vitamin; }
        for (enum Item feather = ITEM_HEALTH_FEATHER; feather <= ITEM_SWIFT_FEATHER; feather++)
            PARAMETRIZE { enabled = rule; item = feather; }
        for (enum Item mochi = ITEM_HEALTH_MOCHI; mochi <= ITEM_SWIFT_MOCHI; mochi++)
            PARAMETRIZE { enabled = rule; item = mochi; }
    }

    if (enabled)
        FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    else
        FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    CreateRandomMonWithIVs(&mon, SPECIES_WOBBUFFET, 50, 0);
    SetMonData(&mon, MON_DATA_FRIENDSHIP, &friendship);
    for (u32 stat = 0; stat < NUM_STATS; stat++)
        SetMonData(&mon, MON_DATA_HP_EV + stat, &ev);
    EXPECT_EQ(ExecuteTableBasedItemEffect(&mon, item, 0, 0), enabled);
    if (enabled)
    {
        for (u32 stat = 0; stat < NUM_STATS; stat++)
            EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_EV + stat), 0);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_FRIENDSHIP), friendship);
    }
    else
    {
        EXPECT_GT(GetMonEVCount(&mon), ev * NUM_STATS);
    }
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
}

TEST("No EVs preserves friendship berries and rejects redundant EV resets")
{
    struct Pokemon mon;
    enum Item item = ITEM_NONE;
    u8 ev = 20;
    u8 friendship = 50;

    for (enum Item berry = ITEM_POMEG_BERRY; berry <= ITEM_TAMATO_BERRY; berry++)
        PARAMETRIZE { item = berry; }
    PARAMETRIZE { item = ITEM_FRESH_START_MOCHI; }

    FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    CreateRandomMonWithIVs(&mon, SPECIES_WOBBUFFET, 50, 0);
    SetMonData(&mon, MON_DATA_FRIENDSHIP, &friendship);
    for (u32 stat = 0; stat < NUM_STATS; stat++)
        SetMonData(&mon, MON_DATA_HP_EV + stat, &ev);
    EXPECT_EQ(ExecuteTableBasedItemEffect(&mon, item, 0, 0), item == ITEM_FRESH_START_MOCHI);
    EXPECT_EQ(GetMonEVCount(&mon), 0);
    if (item == ITEM_FRESH_START_MOCHI)
        EXPECT_EQ(GetMonData(&mon, MON_DATA_FRIENDSHIP), friendship);
    else
    {
        EXPECT_LT(GetMonEVCount(&mon), ev * NUM_STATS);
        EXPECT_GT(GetMonData(&mon, MON_DATA_FRIENDSHIP), friendship);
    }
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
}
