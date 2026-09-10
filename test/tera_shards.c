#include "global.h"
#include "tera_shards.h"
#include "battle_util.h"
#include "battle_terastal.h"
#include "daycare.h"
#include "event_data.h"
#include "item.h"
#include "item_use.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "test/test.h"

TEST("Tera shards unlock on the first same type application and consume only successful changes")
{
    struct Pokemon mon;
    u32 type = TYPE_WATER;
    ClearBag();
    FlagClear(FLAG_TERA_ORB_CHARGED);
    CreateRandomMon(&mon, SPECIES_MAGIKARP, 5);
    EXPECT(!GetMonData(&mon, MON_DATA_TERA_UNLOCKED));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_TERA_TYPE), TYPE_WATER);
    EXPECT(CanApplyTeraShard(&mon.box, ITEM_WATER_TERA_SHARD));
    EXPECT(!ApplyTeraShard(&mon, ITEM_WATER_TERA_SHARD));
    EXPECT(!GetMonData(&mon, MON_DATA_TERA_UNLOCKED));
    EXPECT(AddBagItem(ITEM_WATER_TERA_SHARD, 2));
    EXPECT(ApplyTeraShard(&mon, ITEM_WATER_TERA_SHARD));
    EXPECT(GetMonData(&mon, MON_DATA_TERA_UNLOCKED));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_TERA_TYPE), TYPE_WATER);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_WATER_TERA_SHARD), 1);
    EXPECT(!ApplyTeraShard(&mon, ITEM_WATER_TERA_SHARD));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_WATER_TERA_SHARD), 1);
    EXPECT(AddBagItem(ITEM_FIRE_TERA_SHARD, 1));
    EXPECT(ApplyTeraShard(&mon, ITEM_FIRE_TERA_SHARD));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_TERA_TYPE), TYPE_FIRE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_FIRE_TERA_SHARD), 0);
    EXPECT(!FlagGet(FLAG_TERA_ORB_CHARGED));
    EXPECT(AddBagItem(ITEM_TERA_ORB, 1));
    FlagSet(FLAG_TERA_ORB_CHARGED);
    EXPECT(IsTeraOrbCharged());
    EXPECT(GetMonData(&mon, MON_DATA_TERA_UNLOCKED));
    CreateRandomMon(&mon, SPECIES_MAGIKARP, 5);
    EXPECT(!GetMonData(&mon, MON_DATA_TERA_UNLOCKED));
    SetMonData(&mon, MON_DATA_TERA_TYPE, &type);
    EXPECT(!GetMonData(&mon, MON_DATA_TERA_UNLOCKED));
    ClearBag();
    FlagClear(FLAG_TERA_ORB_CHARGED);
}

TEST("Tera shards cover all nineteen types through usable party items and reject Eggs and non shards")
{
    struct Pokemon mon;
    bool8 seen[NUMBER_OF_MON_TYPES] = {0};
    u32 count = 0;
    ClearBag();
    for (u32 item = 1; item < ITEMS_COUNT; item++)
    {
        if (gItemsInfo[item].sortType != ITEM_TYPE_TERA_SHARD)
            continue;
        u32 type = GetItemSecondaryId(item);
        EXPECT(type > TYPE_NONE && type < NUMBER_OF_MON_TYPES && type != TYPE_MYSTERY);
        EXPECT(!seen[type]);
        seen[type] = TRUE;
        count++;
        EXPECT_EQ(GetItemType(item), ITEM_USE_PARTY_MENU);
        EXPECT_EQ(GetItemFieldFunc(item), ItemUseOutOfBattle_TeraShard);
        EXPECT(AddBagItem(item, 2));
        CreateEgg(&mon, SPECIES_MAGIKARP, FALSE);
        EXPECT(!ApplyTeraShard(&mon, item));
        EXPECT(!GetMonData(&mon, MON_DATA_TERA_UNLOCKED));
        EXPECT_EQ(CountTotalItemQuantityInBag(item), 2);
        CreateRandomMon(&mon, SPECIES_MAGIKARP, 5);
        EXPECT(ApplyTeraShard(&mon, item));
        EXPECT_EQ(GetMonData(&mon, MON_DATA_TERA_TYPE), type);
        EXPECT(GetMonData(&mon, MON_DATA_TERA_UNLOCKED));
    }
    EXPECT_EQ(count, 19);
    EXPECT(!CanApplyTeraShard(&mon.box, ITEM_POTION));
    EXPECT(!CanApplyTeraShard(&mon.box, ITEMS_COUNT));
    ZeroMonData(&mon);
    EXPECT(!CanApplyTeraShard(&mon.box, ITEM_FIRE_TERA_SHARD));
    ClearBag();
}

TEST("Tera shards respect every enabled forced type species without consuming incompatible shards")
{
    struct Pokemon mon;
    ClearBag();
    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        u32 forceType = gSpeciesInfo[species].forceTeraType;
        u32 matchingItem = ITEM_NONE, wrongItem;
        if (!gSpeciesInfo[species].baseHP || !forceType)
            continue;
        for (u32 item = 1; item < ITEMS_COUNT; item++)
            if (gItemsInfo[item].sortType == ITEM_TYPE_TERA_SHARD && GetItemSecondaryId(item) == forceType)
                matchingItem = item;
        EXPECT_NE(matchingItem, ITEM_NONE);
        wrongItem = forceType == TYPE_FIRE ? ITEM_WATER_TERA_SHARD : ITEM_FIRE_TERA_SHARD;
        ClearBag();
        EXPECT(AddBagItem(wrongItem, 1));
        EXPECT(AddBagItem(matchingItem, 2));
        CreateRandomMon(&mon, species, 50);
        EXPECT(!ApplyTeraShard(&mon, wrongItem));
        EXPECT(!GetMonData(&mon, MON_DATA_TERA_UNLOCKED));
        EXPECT_EQ(CountTotalItemQuantityInBag(wrongItem), 1);
        EXPECT(ApplyTeraShard(&mon, matchingItem));
        EXPECT(GetMonData(&mon, MON_DATA_TERA_UNLOCKED));
        EXPECT_EQ(GetMonData(&mon, MON_DATA_TERA_TYPE), forceType);
        EXPECT(!ApplyTeraShard(&mon, matchingItem));
        EXPECT_EQ(CountTotalItemQuantityInBag(matchingItem), 1);
    }
    ClearBag();
}

TEST("Tera shards preserve an individual unlock through species personality copies boxing and save load")
{
    struct Pokemon mon, copy;
    u16 species = SPECIES_GYARADOS;
    struct BoxPokemon *boxed = GetBoxedMonPtr(0, 0);
    struct BoxPokemon saved = *boxed;
    ClearBag();
    CreateRandomMon(&mon, SPECIES_MAGIKARP, 20);
    EXPECT(AddBagItem(ITEM_FIRE_TERA_SHARD, 1));
    EXPECT(ApplyTeraShard(&mon, ITEM_FIRE_TERA_SHARD));
    SetMonData(&mon, MON_DATA_SPECIES, &species);
    CalculateMonStats(&mon);
    EXPECT(GetMonData(&mon, MON_DATA_TERA_UNLOCKED));
    UpdateMonPersonality(&mon.box, GetMonData(&mon, MON_DATA_PERSONALITY) ^ 12345);
    EXPECT(GetMonData(&mon, MON_DATA_TERA_UNLOCKED));
    CopyMon(&copy, &mon, sizeof(copy));
    EXPECT(GetMonData(&copy, MON_DATA_TERA_UNLOCKED));
    CopyMon(boxed, &copy.box, sizeof(*boxed));
    EXPECT(GetBoxMonData(boxed, MON_DATA_TERA_UNLOCKED));
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    ZeroBoxMonData(boxed);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    boxed = GetBoxedMonPtr(0, 0);
    EXPECT(GetBoxMonData(boxed, MON_DATA_TERA_UNLOCKED));
    EXPECT_EQ(GetBoxMonData(boxed, MON_DATA_TERA_TYPE), TYPE_FIRE);
    *boxed = saved;
    ClearBag();
}
