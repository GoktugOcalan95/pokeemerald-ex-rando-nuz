#include "global.h"
#include "slateport_shops.h"
#include "battle_z_move.h"
#include "event_data.h"
#include "item.h"
#include "no_evs.h"
#include "pokemon.h"
#include "save.h"
#include "string_util.h"
#include "test/test.h"

static bool32 StockContains(u32 category, u16 item)
{
    u16 stock[ITEMS_COUNT];
    u32 count = BuildSlateportShopStock(category, stock);
    for (u32 i = 0; i < count; i++)
        if (stock[i] == item)
            return TRUE;
    return FALSE;
}

TEST("Slateport shops sell working evolution and form items")
{
    struct Pokemon mon;
    struct FormChangeContext ctx = {0};
    bool32 canStop = TRUE;
    ClearBag();
    FlagClear(FLAG_IS_CHAMPION);
    EXPECT(TryGiveSlateportPurchase(SLATEPORT_SHOP_TM, ITEM_THUNDER_STONE, 1));
    CreateRandomMon(&mon, SPECIES_PIKACHU, 20);
    EXPECT_EQ(GetEvolutionTargetSpecies(&mon, EVO_MODE_ITEM_USE, ITEM_THUNDER_STONE, NULL, &canStop, CHECK_EVO), SPECIES_RAICHU);
    EXPECT(TryGiveSlateportPurchase(SLATEPORT_SHOP_FORMS, ITEM_ADAMANT_CRYSTAL, 1));
    ctx.currentSpecies = SPECIES_DIALGA;
    ctx.method = FORM_CHANGE_ITEM_HOLD;
    ctx.heldItem = ITEM_ADAMANT_CRYSTAL;
    EXPECT_EQ(GetFormChangeTargetSpecies_Internal(ctx), SPECIES_DIALGA_ORIGIN);
    EXPECT(TryGiveSlateportPurchase(SLATEPORT_SHOP_FORMS, ITEM_FIRE_MEMORY, 1));
    ctx.currentSpecies = SPECIES_SILVALLY;
    ctx.heldItem = ITEM_FIRE_MEMORY;
    ctx.ability = ABILITY_RKS_SYSTEM;
    EXPECT_EQ(GetFormChangeTargetSpecies_Internal(ctx), SPECIES_SILVALLY_FIRE);
    FlagSet(FLAG_IS_CHAMPION);
    EXPECT(TryGiveSlateportPurchase(SLATEPORT_SHOP_MEGA, ITEM_VENUSAURITE, 1));
    ctx.currentSpecies = SPECIES_VENUSAUR;
    ctx.method = FORM_CHANGE_BATTLE_MEGA_EVOLUTION_ITEM;
    ctx.heldItem = ITEM_VENUSAURITE;
    EXPECT_EQ(GetFormChangeTargetSpecies_Internal(ctx), SPECIES_VENUSAUR_MEGA);
    EXPECT(TryGiveSlateportPurchase(SLATEPORT_SHOP_Z, ITEM_PIKANIUM_Z, 1));
    EXPECT_EQ(GetSignatureZMove(MOVE_VOLT_TACKLE, SPECIES_PIKACHU, ITEM_PIKANIUM_Z), MOVE_CATASTROPIKA);
    ClearBag();
    FlagClear(FLAG_IS_CHAMPION);
}

TEST("Slateport shops gate complete postgame categories and alphabetize every purchase list")
{
    u16 stock[ITEMS_COUNT];
    const u16 postgameCounts[] = {92, 35, 19, 21};
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    FlagClear(FLAG_IS_CHAMPION);
    for (u32 category = SLATEPORT_SHOP_MEGA; category < SLATEPORT_SHOP_COUNT; category++)
        EXPECT_EQ(BuildSlateportShopStock(category, stock), 0);
    EXPECT_EQ(BuildSlateportShopStock(SLATEPORT_SHOP_ENERGY, stock), 15);
    FlagSet(FLAG_IS_CHAMPION);
    for (u32 category = 0; category < SLATEPORT_SHOP_COUNT; category++)
    {
        u32 count = BuildSlateportShopStock(category, stock);
        EXPECT_GT(count, 0);
        EXPECT_EQ(stock[count], ITEM_NONE);
        if (category >= SLATEPORT_SHOP_MEGA)
            EXPECT_EQ(count, postgameCounts[category - SLATEPORT_SHOP_MEGA]);
        for (u32 i = 0; i < count; i++)
        {
            EXPECT_NE(stock[i], ITEM_ABILITY_CAPSULE);
            EXPECT_NE(stock[i], ITEM_ABILITY_PATCH);
            if (category >= SLATEPORT_SHOP_MEGA)
                EXPECT(!IsSlateportPreChampionItem(stock[i]));
            if (i > 0)
            {
                u8 previous[ITEM_NAME_LENGTH + 1], current[ITEM_NAME_LENGTH + 1];
                StringCopyUppercase(previous, GetItemName(stock[i - 1]));
                StringCopyUppercase(current, GetItemName(stock[i]));
                EXPECT_LE(StringCompare(previous, current), 0);
                EXPECT_NE(stock[i - 1], stock[i]);
            }
        }
    }
    FlagClear(FLAG_IS_CHAMPION);
}

TEST("Slateport shops cover functional evolution form and legendary items before Champion")
{
    const u16 forms[] = {ITEM_RED_ORB, ITEM_BLUE_ORB, ITEM_RUSTED_SWORD, ITEM_RUSTED_SHIELD,
        ITEM_WELLSPRING_MASK, ITEM_HEARTHFLAME_MASK, ITEM_CORNERSTONE_MASK,
        ITEM_ROTOM_CATALOG, ITEM_ZYGARDE_CUBE, ITEM_DNA_SPLICERS, ITEM_N_SOLARIZER, ITEM_N_LUNARIZER,
        ITEM_REINS_OF_UNITY, ITEM_ADAMANT_CRYSTAL, ITEM_LUSTROUS_GLOBE, ITEM_GRISEOUS_CORE,
        ITEM_FIST_PLATE, ITEM_FAIRY_MEMORY, ITEM_DOUSE_DRIVE, ITEM_RED_NECTAR};
    const u16 evolution[] = {ITEM_LINKING_CORD, ITEM_METAL_COAT, ITEM_UPGRADE, ITEM_DUBIOUS_DISC,
        ITEM_AUSPICIOUS_ARMOR, ITEM_MALICIOUS_ARMOR, ITEM_MASTERPIECE_TEACUP, ITEM_GIMMIGHOUL_COIN,
        ITEM_STRAWBERRY_SWEET, ITEM_RIBBON_SWEET, ITEM_METAL_ALLOY};
    FlagClear(FLAG_IS_CHAMPION);
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    for (u32 i = 0; i < ARRAY_COUNT(forms); i++)
    {
        EXPECT(StockContains(SLATEPORT_SHOP_FORMS, forms[i]));
        EXPECT(IsSlateportPreChampionItem(forms[i]));
    }
    for (u32 i = 0; i < ARRAY_COUNT(evolution); i++)
        EXPECT(StockContains(SLATEPORT_SHOP_EVOLUTION, evolution[i]));
    EXPECT(StockContains(SLATEPORT_SHOP_TM, ITEM_THUNDER_STONE));
    EXPECT(StockContains(SLATEPORT_SHOP_TM, ITEM_SEA_INCENSE));
    EXPECT(StockContains(SLATEPORT_SHOP_TM, ITEM_TM_HIDDEN_POWER));
    EXPECT(StockContains(SLATEPORT_SHOP_TM, ITEM_TM_SECRET_POWER));
    EXPECT(!IsSlateportPreChampionItem(ITEM_POKE_BALL));
    EXPECT_EQ(GetItemSellPrice(ITEM_POKE_BALL), GetItemPrice(ITEM_POKE_BALL) / ITEM_SELL_FACTOR);
}

TEST("Slateport shops fix special prices against discounts and prevent resale profit")
{
    u16 stock[ITEMS_COUNT];
    FlagSet(FLAG_IS_CHAMPION);
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    for (u32 category = 0; category < SLATEPORT_SHOP_COUNT; category++)
    {
        u32 count = BuildSlateportShopStock(category, stock);
        for (u32 i = 0; i < count; i++)
        {
            u16 item = stock[i];
            if (item == ITEM_MEGA_RING || item == ITEM_Z_POWER_RING || item == ITEM_TERA_ORB)
            {
                EXPECT_EQ(GetSlateportItemPrice(item, FALSE), 10000);
                EXPECT_EQ(GetSlateportItemPrice(item, TRUE), 10000);
                EXPECT_EQ(GetItemPrice(item), 0);
            }
            else if (category == SLATEPORT_SHOP_ENERGY || GetItemPocket(item) == POCKET_TM_HM)
            {
                EXPECT_EQ(GetSlateportItemPrice(item, FALSE), GetItemPrice(item));
                EXPECT_EQ(GetSlateportItemPrice(item, TRUE), GetItemPrice(item) / 2);
            }
            else
            {
                EXPECT_EQ(GetSlateportItemPrice(item, FALSE), 100);
                EXPECT_EQ(GetSlateportItemPrice(item, TRUE), 100);
                EXPECT_LE(GetItemSellPrice(item), 50);
            }
        }
    }
    EXPECT_EQ(GetItemSellPrice(ITEM_POTION), GetItemPrice(ITEM_POTION) / ITEM_SELL_FACTOR);
    FlagClear(FLAG_IS_CHAMPION);
}

TEST("Slateport shops grant activation keys separately once and persist charged Orb ownership")
{
    const u16 keys[] = {ITEM_MEGA_RING, ITEM_Z_POWER_RING, ITEM_TERA_ORB};
    ClearBag();
    memset(gSaveBlock1Ptr->pcItems, 0, sizeof(gSaveBlock1Ptr->pcItems));
    FlagClear(FLAG_TERA_ORB_CHARGED);
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    for (u32 i = 0; i < ARRAY_COUNT(keys); i++)
    {
        EXPECT(!TryGiveSlateportPurchase(SLATEPORT_SHOP_ENERGY, keys[i], 2));
        EXPECT(TryGiveSlateportPurchase(SLATEPORT_SHOP_ENERGY, keys[i], 1));
        EXPECT(!TryGiveSlateportPurchase(SLATEPORT_SHOP_ENERGY, keys[i], 1));
        EXPECT_EQ(CountTotalItemQuantityInBag(keys[i]), 1);
        EXPECT_EQ(FlagGet(FLAG_TERA_ORB_CHARGED), keys[i] == ITEM_TERA_ORB);
    }
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    ClearBag();
    FlagClear(FLAG_TERA_ORB_CHARGED);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT(FlagGet(FLAG_TERA_ORB_CHARGED));
    EXPECT(!TryGiveSlateportPurchase(SLATEPORT_SHOP_ENERGY, ITEM_TERA_ORB, 1));
    EXPECT(RemoveBagItem(ITEM_MEGA_RING, 1));
    EXPECT(AddPCItem(ITEM_MEGA_RING, 1));
    EXPECT(!TryGiveSlateportPurchase(SLATEPORT_SHOP_ENERGY, ITEM_MEGA_RING, 1));
    ClearBag();
    memset(gSaveBlock1Ptr->pcItems, 0, sizeof(gSaveBlock1Ptr->pcItems));
    FlagClear(FLAG_TERA_ORB_CHARGED);
}

TEST("Slateport shops reject full bags unavailable categories and No EV stock without side effects")
{
    u16 stock[ITEMS_COUNT];
    ClearBag();
    FlagClear(FLAG_IS_CHAMPION);
    FlagClear(FLAG_TERA_ORB_CHARGED);
    FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    EXPECT_EQ(BuildSlateportShopStock(SLATEPORT_SHOP_ENERGY, stock), 3);
    EXPECT(!TryGiveSlateportPurchase(SLATEPORT_SHOP_ENERGY, ITEM_HP_UP, 1));
    EXPECT(!TryGiveSlateportPurchase(SLATEPORT_SHOP_MEGA, ITEM_VENUSAURITE, 1));
    EXPECT(!TryGiveSlateportPurchase(SLATEPORT_SHOP_TERA, ITEM_FIRE_TERA_SHARD, 1));
    EXPECT(!TryGiveSlateportPurchase(SLATEPORT_SHOP_COUNT, ITEM_TERA_ORB, 1));
    for (u32 slot = 0; slot < gBagPockets[BAG_KEY_ITEMS].capacity; slot++)
        EXPECT(AddBagItem(ITEM_TOGGLE_REPEL, MAX_BAG_ITEM_CAPACITY));
    EXPECT(!CheckBagHasSpace(ITEM_TERA_ORB, 1));
    EXPECT(!TryGiveSlateportPurchase(SLATEPORT_SHOP_ENERGY, ITEM_TERA_ORB, 1));
    EXPECT(!FlagGet(FLAG_TERA_ORB_CHARGED));
    ClearBag();
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
}
