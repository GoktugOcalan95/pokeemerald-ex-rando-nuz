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
#include "script.h"
#include "constants/script_commands.h"
#include "constants/script_menu.h"

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
    const u16 postgameCounts[] = {92, 35, 19, 21, 18};
    FlagSet(FLAG_RUN_RULE_LIMIT_SLATEPORT_SHOP);
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
    FlagSet(FLAG_RUN_RULE_LIMIT_SLATEPORT_SHOP);
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

TEST("Slateport shops limit all five special categories independently of item randomization")
{
    const u16 examples[] = {ITEM_VENUSAURITE, ITEM_PIKANIUM_Z, ITEM_FIRE_TERA_SHARD, ITEM_ADAMANT_MINT, ITEM_FIRE_GEM};
    const u16 counts[] = {92, 35, 19, 21, 18};
    u16 stock[ITEMS_COUNT];

    for (u32 mask = 0; mask < 8; mask++)
    {
        InitEventData();
        if (mask & 1)
            FlagSet(FLAG_RUN_RULE_LIMIT_SLATEPORT_SHOP);
        if (mask & 2)
            FlagSet(FLAG_IS_CHAMPION);
        if (mask & 4)
            FlagSet(FLAG_RUN_RULE_ITEMS);
        bool32 available = !(mask & 1) || (mask & 2);
        CheckSlateportSpecialStock();
        EXPECT_EQ(gSpecialVar_Result, available != FALSE);
        for (u32 i = 0; i < ARRAY_COUNT(examples); i++)
        {
            ClearBag();
            EXPECT_EQ(BuildSlateportShopStock(SLATEPORT_SHOP_MEGA + i, stock), available ? counts[i] : 0);
            EXPECT_EQ(TryGiveSlateportPurchase(SLATEPORT_SHOP_MEGA + i, examples[i], 1), available != FALSE);
            EXPECT(!IsSlateportPreChampionItem(examples[i]));
        }
        EXPECT(StockContains(SLATEPORT_SHOP_ENERGY, ITEM_HP_UP));
        EXPECT(StockContains(SLATEPORT_SHOP_ENERGY, ITEM_TERA_ORB));
        EXPECT(StockContains(SLATEPORT_SHOP_FORMS, ITEM_ADAMANT_CRYSTAL));
        FlagSet(FLAG_IS_CHAMPION);
        FlagSet(FLAG_RUN_RULE_BAN_SLATEPORT);
        FlagSet(FLAG_RUN_RULE_BAN_MEGA_STONES);
        FlagSet(FLAG_RUN_RULE_BAN_Z_CRYSTALS);
        FlagSet(FLAG_RUN_RULE_BAN_TERA_SHARDS);
        FlagSet(FLAG_RUN_RULE_BAN_TYPE_GEMS);
        FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
        for (u32 i = 0; i < ARRAY_COUNT(examples); i++)
            EXPECT(StockContains(SLATEPORT_SHOP_MEGA + i, examples[i]));
    }
    ClearBag();
    InitEventData();
}


extern const u8 SlateportCity_EventScript_EnergyGuru[];
extern const u8 SlateportCity_EventScript_FormShop[];
extern ScrCmdFunc gScriptCmdTable[];
extern ScrCmdFunc gScriptCmdTableEnd[];
static EWRAM_DATA u32 sShopMenuCount;
static EWRAM_DATA u32 sShopMenuId;
static EWRAM_DATA u32 sShopChoice;
static EWRAM_DATA u32 sShopOpened;
static EWRAM_DATA u32 sShopCategory;

static bool8 SkipShopPresentation(struct ScriptContext *ctx)
{
    if (ctx->scriptPtr[-1] == SCR_OP_MESSAGE)
        ctx->scriptPtr += 4;
    else if (ctx->scriptPtr[-1] == SCR_OP_CALL_STD)
        ctx->scriptPtr++;
    return TRUE;
}

static bool8 ChooseShopCategory(struct ScriptContext *ctx)
{
    ctx->scriptPtr += 2;
    sShopMenuId = ScriptReadByte(ctx);
    ctx->scriptPtr++;
    gSpecialVar_Result = sShopMenuCount++ == 0 ? sShopChoice : 127;
    return TRUE;
}

static bool8 CaptureShopNative(struct ScriptContext *ctx)
{
    u32 pointer = ScriptReadWord(ctx) & ~0x02000000;
    if (pointer == (u32)OpenSlateportShop)
    {
        sShopOpened++;
        sShopCategory = gSpecialVar_0x8004;
        gSpecialVar_Result = TRUE;
        return TRUE;
    }
    ctx->scriptPtr -= 4;
    return gScriptCmdTable[SCR_OP_CALLNATIVE](ctx);
}

TEST("Slateport shops route sellers directly when locked and offer the correct unlocked categories")
{
    ScrCmdFunc commands[256];
    u32 commandCount = gScriptCmdTableEnd - gScriptCmdTable;
    const u8 presentation[] = {SCR_OP_MESSAGE, SCR_OP_WAITMESSAGE, SCR_OP_CALL_STD,
        SCR_OP_LOCK, SCR_OP_RELEASE, SCR_OP_FACEPLAYER, SCR_OP_WAITSTATE};
    const u8 *const scripts[] = {SlateportCity_EventScript_EnergyGuru, SlateportCity_EventScript_FormShop};
    const u8 formCategories[] = {SLATEPORT_SHOP_FORMS, SLATEPORT_SHOP_MEGA, SLATEPORT_SHOP_Z,
        SLATEPORT_SHOP_TERA, SLATEPORT_SHOP_GEMS};

    memcpy(commands, gScriptCmdTable, commandCount * sizeof(*commands));
    for (u32 i = 0; i < ARRAY_COUNT(presentation); i++)
        commands[presentation[i]] = SkipShopPresentation;
    commands[SCR_OP_MULTICHOICE] = ChooseShopCategory;
    commands[SCR_OP_CALLNATIVE] = CaptureShopNative;
    for (u32 mask = 0; mask < 4; mask++)
        for (u32 seller = 0; seller < ARRAY_COUNT(scripts); seller++)
            for (u32 choice = 0; choice < (seller ? 6 : 3); choice++)
            {
                struct ScriptContext ctx;
                InitEventData();
                if (mask & 1)
                    FlagSet(FLAG_RUN_RULE_LIMIT_SLATEPORT_SHOP);
                if (mask & 2)
                    FlagSet(FLAG_IS_CHAMPION);
                bool32 available = !(mask & 1) || (mask & 2);
                bool32 cancel = choice == (seller ? 5 : 2);
                sShopChoice = choice;
                sShopMenuCount = sShopOpened = 0;
                InitScriptContext(&ctx, commands, commands + commandCount);
                SetupBytecodeScript(&ctx, scripts[seller]);
                u32 steps = 0;
                while (RunScriptCommand(&ctx))
                    EXPECT(++steps < 200);
                EXPECT_EQ(sShopMenuCount, available ? (cancel ? 1 : 2) : 0);
                EXPECT_EQ(sShopOpened, available && cancel ? 0 : 1);
                if (available)
                    EXPECT_EQ(sShopMenuId, seller ? MULTI_SLATEPORT_STOCK : MULTI_SLATEPORT_SUPPLIES);
                if (sShopOpened)
                    EXPECT_EQ(sShopCategory, !available ? (seller ? SLATEPORT_SHOP_FORMS : SLATEPORT_SHOP_ENERGY)
                        : seller ? formCategories[choice] : choice ? SLATEPORT_SHOP_MINTS : SLATEPORT_SHOP_ENERGY);
            }
    InitEventData();
}
