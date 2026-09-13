#include "global.h"
#include "gba/flash_internal.h"
#include "bag_categories.h"
#include "item.h"
#include "text.h"
#include "strings.h"
#include "string_util.h"
#include "item_menu.h"
#include "list_menu.h"
#include "bg.h"
#include "dma3.h"
#include "malloc.h"
#include "window.h"
#include "load_save.h"
#include "save.h"
#include "pokemon_storage_system.h"
#include "test/test.h"

static void FillExpandedBag(u32 quantity)
{
    ClearBag();
    SetBagItemsPointers();
    for (u32 item = 1; item < ITEMS_COUNT; item++)
        if (GetBagStoragePocket(item) < BAG_POCKETS_COUNT)
            EXPECT(AddBagItem(item, quantity));
}

static void CheckExpandedBag(u32 quantity)
{
    for (u32 item = 1; item < ITEMS_COUNT; item++)
        if (GetBagStoragePocket(item) < BAG_POCKETS_COUNT)
            EXPECT_EQ(CountTotalItemQuantityInBag(item), quantity);
}

TEST("Expanded Bag holds every distinct item at 999 and sorts all eleven categories")
{
    FillExpandedBag(999);
    CheckExpandedBag(999);
    for (u32 pocket = 0; pocket < BAG_POCKETS_COUNT; pocket++)
    {
        EXPECT(IsBagPocketNonEmpty(pocket));
        SortItemsInBag(&gBagPockets[pocket], SORT_ALPHABETICALLY);
        for (u32 slot = 0; slot < gBagPockets[pocket].capacity; slot++)
        {
            struct ItemSlot item = BagPocket_GetSlotData(&gBagPockets[pocket], slot);
            EXPECT_EQ(GetBagStoragePocket(item.itemId), pocket);
            EXPECT_EQ(item.quantity, 999);
            EXPECT(!CheckBagHasSpace(item.itemId, 1));
        }
    }
    CheckExpandedBag(999);
    for (u32 item = 1; item < ITEMS_COUNT; item++)
        if (GetBagStoragePocket(item) < BAG_POCKETS_COUNT)
            EXPECT(RemoveBagItem(item, 998));
    CheckExpandedBag(1);
    ClearBag();
    for (u32 pocket = 0; pocket < BAG_POCKETS_COUNT; pocket++)
        EXPECT(!IsBagPocketNonEmpty(pocket));
}

TEST("Expanded Bag preserves both packed regions through encryption and temporary backup")
{
    FillExpandedBag(731);
    u32 oldKey = gSaveBlock2Ptr->encryptionKey;
    ApplyNewEncryptionKeyToBagItems(0xABCDEF98);
    gSaveBlock2Ptr->encryptionKey = 0xABCDEF98;
    CheckExpandedBag(731);
    LoadPlayerBag();
    ClearBag();
    gSaveBlock2Ptr->encryptionKey = 0x12345678;
    SavePlayerBag();
    CheckExpandedBag(731);
    ApplyNewEncryptionKeyToBagItems(oldKey);
    gSaveBlock2Ptr->encryptionKey = oldKey;
    CheckExpandedBag(731);
    ClearBag();
}

TEST("Expanded Bag round trips every item through all ordinary save paths")
{
    u32 mode;
    PARAMETRIZE { mode = 0; }
    PARAMETRIZE { mode = 1; }
    PARAMETRIZE { mode = 2; }
    PARAMETRIZE { mode = 3; }
    PARAMETRIZE { mode = 4; }
    gTestRunnerState.timeoutSeconds = 120;
    ClearSaveData();
    Save_ResetSaveCounters();
    FillExpandedBag(111);
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    u32 quantity = 123 + mode;
    FillExpandedBag(quantity);
    gPokemonStoragePtr->boxNames[13][0] = 51 + mode;
    if (mode < 3)
    {
        const u32 types[] = {SAVE_NORMAL, SAVE_LINK, SAVE_EREADER};
        EXPECT_EQ(TrySavingData(types[mode]), SAVE_STATUS_OK);
    }
    else if (mode == 3)
    {
        EXPECT(!WriteSaveBlock2());
        u32 calls = 0;
        while (!WriteSaveBlock1Sector() && calls < 20)
            calls++;
        EXPECT_EQ(calls, NUM_SECTORS_PER_SLOT - 1);
    }
    else
    {
        EXPECT(!LinkFullSave_Init());
        u32 calls = 0;
        while (!LinkFullSave_WriteSector() && calls < 20)
            calls++;
        EXPECT(calls < 20);
        LinkFullSave_ReplaceLastSector();
        LinkFullSave_SetLastSectorSignature();
    }
    ClearBag();
    gPokemonStoragePtr->boxNames[13][0] = 0;
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    SetBagItemsPointers();
    CheckExpandedBag(quantity);
    EXPECT_EQ(gPokemonStoragePtr->boxNames[13][0], 51 + mode);
    ClearBag();
}

TEST("Expanded Bag classifies functional evolution equipment without changing gameplay pockets")
{
    EXPECT_EQ(GetBagStoragePocket(ITEM_LEFTOVERS), BAG_HELD_ITEMS);
    EXPECT_EQ(GetBagStoragePocket(ITEM_POWER_BRACER), BAG_HELD_ITEMS);
    EXPECT_EQ(GetBagStoragePocket(ITEM_METAL_COAT), BAG_EVOLUTION_FORMS);
    EXPECT_EQ(GetBagStoragePocket(ITEM_RUSTED_SWORD), BAG_EVOLUTION_FORMS);
    EXPECT_EQ(GetBagStoragePocket(ITEM_ABILITY_PATCH), BAG_TRAINING);
    EXPECT_EQ(GetBagStoragePocket(ITEM_PP_UP), BAG_TRAINING);
    EXPECT_EQ(GetBagStoragePocket(ITEM_MAX_ETHER), BAG_MEDICINE);
    EXPECT_EQ(GetBagStoragePocket(ITEM_REPEL), BAG_OTHER_ITEMS);
    EXPECT_EQ(GetBagStoragePocket(ITEM_NUGGET), BAG_TREASURES);
    EXPECT_EQ(GetBagStoragePocket(ITEM_X_ATTACK), BAG_BATTLE_ITEMS);
    EXPECT_EQ(GetItemPocket(ITEM_METAL_COAT), POCKET_ITEMS);
    EXPECT_EQ(GetItemPocket(ITEM_ORAN_BERRY), POCKET_BERRIES);
}

TEST("Expanded Bag moves items beyond slot 255 without losing adjacent stacks")
{
    FillExpandedBag(513);
    bool32 checked = FALSE;
    for (u32 pocket = 0; pocket < BAG_POCKETS_COUNT; pocket++)
    {
        if (gBagPockets[pocket].capacity <= 256)
            continue;
        u32 last = gBagPockets[pocket].capacity - 1;
        enum Item firstItem = GetBagItemId(pocket, 0);
        enum Item lastItem = GetBagItemId(pocket, last);
        MoveItemSlotInPocket(pocket, last, 0);
        EXPECT_EQ(GetBagItemId(pocket, 0), lastItem);
        EXPECT_EQ(GetBagItemId(pocket, 1), firstItem);
        MoveItemSlotInPocket(pocket, 0, last + 1);
        EXPECT_EQ(GetBagItemId(pocket, last), lastItem);
        EXPECT_EQ(GetBagItemId(pocket, 0), firstItem);
        checked = TRUE;
    }
    EXPECT(checked);
    CheckExpandedBag(513);
    ClearBag();
}

TEST("Expanded Bag detects damaged extension bytes and rejects invalid sector IDs")
{
    gTestRunnerState.timeoutSeconds = 120;
    for (u32 corruptId = 0; corruptId < 2; corruptId++)
    {
        Save_ResetSaveCounters();
        FillExpandedBag(212);
        EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
        FillExpandedBag(313);
        EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
        u32 sector = (gSaveCounter % NUM_SAVE_SLOTS) * NUM_SECTORS_PER_SLOT;
        ReadFlash(sector, 0, (u8 *)&gSaveDataBuffer, sizeof(gSaveDataBuffer));
        if (corruptId)
            gSaveDataBuffer.id = 0xFFFF;
        else
            gSaveDataBuffer.saveBlock3Chunk[0] ^= 1;
        EXPECT_EQ(ProgramFlashSector(sector, (u8 *)&gSaveDataBuffer), 0);
        ClearBag();
        EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_ERROR);
        CheckExpandedBag(212);
    }
    ClearBag();
}

u32 Test_MoveBagPocketChoice(u32 current, u16 keys);
u32 Test_ChangeBagPocket(u32 current, s32 delta);
const u8 *Test_GetBagCategoryName(u32 pocket);

TEST("Expanded Bag chooser follows row major directions and pockets wrap in both directions")
{
    for (u32 pocket = 0; pocket < BAG_POCKETS_COUNT; pocket++)
    {
        EXPECT_EQ(Test_ChangeBagPocket(pocket, 1), (pocket + 1) % BAG_POCKETS_COUNT);
        EXPECT_EQ(Test_ChangeBagPocket(pocket, -1), (pocket + BAG_POCKETS_COUNT - 1) % BAG_POCKETS_COUNT);
        EXPECT_EQ(Test_MoveBagPocketChoice(pocket, DPAD_UP), pocket >= 2 ? pocket - 2 : pocket);
        EXPECT_EQ(Test_MoveBagPocketChoice(pocket, DPAD_DOWN), pocket + 2 < BAG_POCKETS_COUNT ? pocket + 2 : pocket);
        EXPECT_EQ(Test_MoveBagPocketChoice(pocket, DPAD_LEFT), pocket % 2 ? pocket - 1 : pocket);
        EXPECT_EQ(Test_MoveBagPocketChoice(pocket, DPAD_RIGHT), pocket % 2 == 0 && pocket + 1 < BAG_POCKETS_COUNT ? pocket + 1 : pocket);
        EXPECT_EQ(Test_MoveBagPocketChoice(pocket, B_BUTTON), pocket);
        EXPECT(GetStringWidth(FONT_NARROW, Test_GetBagCategoryName(pocket), 0) <= 104);
        EXPECT(GetStringWidth(FONT_SMALL, gPocketNamesStringsTable[pocket], 0) <= 64);
    }
}

TEST("Expanded Bag removes quantities across multiple stacks without discarding the remainder")
{
    ClearBag();
    EXPECT(AddBagItem(ITEM_POTION, 2500));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), 2500);
    EXPECT(RemoveBagItem(ITEM_POTION, 1500));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), 1000);
    EXPECT(!RemoveBagItem(ITEM_POTION, 1001));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), 1000);
    EXPECT(RemoveBagItem(ITEM_POTION, 1000));
    EXPECT(!CheckBagHasItem(ITEM_POTION, 1));
    ClearBag();
}

TEST("Expanded Bag counts and removes more than 65535 items across stacks")
{
    ClearBag();
    struct BagPocket *pocket = &gBagPockets[GetBagStoragePocket(ITEM_LEFTOVERS)];
    EXPECT(pocket->capacity > 66);
    for (u32 i = 0; i < pocket->capacity; i++)
        BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_LEFTOVERS, 999);
    u32 total = pocket->capacity * 999;
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_LEFTOVERS), total);
    EXPECT(RemoveBagItem(ITEM_LEFTOVERS, total - 123));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_LEFTOVERS), 123);
    ClearBag();
}

void Test_SortBagDisplay(struct ListMenuItem *items, u32 count, u8 pocket, enum BagSortOptions mode);

TEST("Expanded Bag display sorting preserves inventory and targets the selected stack")
{
    struct ListMenuItem display[4] = {0};
    const u16 items[] = {ITEM_POTION, ITEM_ANTIDOTE, ITEM_FULL_RESTORE};
    const u16 quantities[] = {3, 7, 2};
    ClearBag();
    for (u32 i = 0; i < ARRAY_COUNT(items); i++)
    {
        EXPECT_EQ(GetBagStoragePocket(items[i]), BAG_MEDICINE);
        EXPECT(AddBagItem(items[i], quantities[i]));
        display[i].id = i;
        display[i].name = GetItemName(items[i]);
    }
    display[3].id = LIST_CANCEL;
    Test_SortBagDisplay(display, 3, BAG_MEDICINE, SORT_ALPHABETICALLY);
    EXPECT_EQ(display[0].id, 1);
    EXPECT_EQ(display[1].id, 2);
    EXPECT_EQ(display[2].id, 0);
    EXPECT_EQ(display[0].name, GetItemName(ITEM_ANTIDOTE));
    EXPECT_EQ(display[3].id, LIST_CANCEL);
    Test_SortBagDisplay(display, 3, BAG_MEDICINE, SORT_BY_AMOUNT);
    EXPECT_EQ(display[0].id, 1);
    EXPECT_EQ(display[1].id, 0);
    EXPECT_EQ(display[2].id, 2);
    for (u32 i = 0; i < ARRAY_COUNT(items); i++)
    {
        EXPECT_EQ(GetBagItemId(BAG_MEDICINE, i), items[i]);
        EXPECT_EQ(GetBagItemQuantity(BAG_MEDICINE, i), quantities[i]);
    }
    RemoveBagItemFromSlot(&gBagPockets[BAG_MEDICINE], display[0].id, 2);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_ANTIDOTE), 5);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), 3);
    ClearBag();
}

TEST("Expanded Bag acquisition order survives holes replenishment and reacquisition")
{
    ClearBag();
    EXPECT(AddBagItem(ITEM_POTION, 3));
    EXPECT(AddBagItem(ITEM_ANTIDOTE, 7));
    EXPECT(AddBagItem(ITEM_FULL_RESTORE, 2));
    RemoveBagItemFromSlot(&gBagPockets[BAG_MEDICINE], 0, 3);
    EXPECT(AddBagItem(ITEM_FULL_RESTORE, 4));
    EXPECT_EQ(GetBagItemId(BAG_MEDICINE, 0), ITEM_ANTIDOTE);
    EXPECT_EQ(GetBagItemId(BAG_MEDICINE, 1), ITEM_FULL_RESTORE);
    EXPECT_EQ(GetBagItemQuantity(BAG_MEDICINE, 1), 6);
    EXPECT_EQ(GetBagItemId(BAG_MEDICINE, 2), ITEM_NONE);
    EXPECT(AddBagItem(ITEM_POTION, 1));
    EXPECT_EQ(GetBagItemId(BAG_MEDICINE, 2), ITEM_POTION);
    EXPECT(AddBagItem(ITEM_ANTIDOTE, 999));
    EXPECT_EQ(GetBagItemId(BAG_MEDICINE, 0), ITEM_ANTIDOTE);
    EXPECT_EQ(GetBagItemQuantity(BAG_MEDICINE, 0), 999);
    EXPECT_EQ(GetBagItemId(BAG_MEDICINE, 3), ITEM_ANTIDOTE);
    EXPECT_EQ(GetBagItemQuantity(BAG_MEDICINE, 3), 7);
    ClearBag();
}

bool32 Test_LoadBagDisplay(u8 pocket, enum BagSortOptions mode, s32 *slots, u32 count);
u32 Test_DrawBagQuickPanel(bool32 chooser);

TEST("Expanded Bag returns to acquired or numerical order and preserves Berry Tag navigation")
{
    s32 slots[4];
    ClearBag();
    EXPECT(AddBagItem(ITEM_POTION, 3));
    EXPECT(AddBagItem(ITEM_ANTIDOTE, 7));
    EXPECT(Test_LoadBagDisplay(BAG_MEDICINE, SORT_ALPHABETICALLY, slots, 3));
    EXPECT_EQ(slots[0], 1);
    EXPECT_EQ(slots[1], 0);
    EXPECT_EQ(slots[2], LIST_CANCEL);
    EXPECT(Test_LoadBagDisplay(BAG_MEDICINE, SORT_BY_ACQUIRED, slots, 3));
    EXPECT_EQ(slots[0], 0);
    EXPECT_EQ(slots[1], 1);
    EXPECT(AddBagItem(ITEM_ORAN_BERRY, 1));
    EXPECT(AddBagItem(ITEM_CHERI_BERRY, 2));
    EXPECT(AddBagItem(ITEM_ASPEAR_BERRY, 3));
    EXPECT(Test_LoadBagDisplay(BAG_BERRIES, SORT_ALPHABETICALLY, slots, 4));
    EXPECT_EQ(GetBagBerryDisplayItem(0), ITEM_ASPEAR_BERRY);
    EXPECT_EQ(GetBagBerryDisplayItem(1), ITEM_CHERI_BERRY);
    EXPECT_EQ(GetBagBerryDisplayItem(2), ITEM_ORAN_BERRY);
    EXPECT_EQ(GetBagBerryDisplayItem(3), ITEM_NONE);
    EXPECT_EQ(GetBagBerryDisplayItem(-1), ITEM_NONE);
    EXPECT(Test_LoadBagDisplay(BAG_BERRIES, SORT_BY_ACQUIRED, slots, 4));
    EXPECT_EQ(GetBagBerryDisplayItem(0), ITEM_CHERI_BERRY);
    EXPECT_EQ(GetBagBerryDisplayItem(1), ITEM_ASPEAR_BERRY);
    EXPECT_EQ(GetBagBerryDisplayItem(2), ITEM_ORAN_BERRY);
    EXPECT(AddBagItem(ITEM_HM01, 1));
    EXPECT(AddBagItem(ITEM_TM02, 2));
    EXPECT(AddBagItem(ITEM_TM01, 3));
    EXPECT(Test_LoadBagDisplay(BAG_TM_HM, SORT_BY_ACQUIRED, slots, 4));
    EXPECT_EQ(GetBagItemId(BAG_TM_HM, slots[0]), ITEM_TM01);
    EXPECT_EQ(GetBagItemId(BAG_TM_HM, slots[1]), ITEM_TM02);
    EXPECT_EQ(GetBagItemId(BAG_TM_HM, slots[2]), ITEM_HM01);
    ClearBag();
}

TEST("Expanded Bag quick panels use a readable palette and nonoverlapping graphics storage")
{
    bool32 chooser;
    PARAMETRIZE { chooser = FALSE; }
    PARAMETRIZE { chooser = TRUE; }
    DeactivateAllTextPrinters();
    ClearDma3Requests();
    ResetBgsAndClearDma3BusyFlags(FALSE);
    SetDefaultFontsPointer();
    gBagMenu = AllocZeroed(sizeof(*gBagMenu));
    EXPECT(gBagMenu != NULL);
    u32 window = Test_DrawBagQuickPanel(chooser);
    EXPECT_NE(window, WINDOW_NONE);
    EXPECT_EQ(GetWindowAttribute(window, WINDOW_BG), 1);
    EXPECT_EQ(GetWindowAttribute(window, WINDOW_PALETTE_NUM), 15);
    u32 base = GetWindowAttribute(window, WINDOW_BASE_BLOCK);
    u32 end = base + GetWindowAttribute(window, WINDOW_WIDTH) * GetWindowAttribute(window, WINDOW_HEIGHT);
    EXPECT_GE(base, 0x21D);
    EXPECT_LE(end, 0x3E0);
    EXPECT_LE(GetStringWidth(FONT_SMALL, COMPOUND_STRING("A: Choose  B: Back"), 0), 118);
    EXPECT_LE(GetStringWidth(FONT_SMALL, COMPOUND_STRING("START: Menu"), 0), 96);
    EXPECT_LE(GetStringWidth(FONT_NORMAL, COMPOUND_STRING("Order resets when"), 0), 109);
    EXPECT_LE(GetStringWidth(FONT_NORMAL, COMPOUND_STRING("you leave the Bag."), 0), 109);
    ProcessDma3Requests();
    DeactivateAllTextPrinters();
    FreeAllWindowBuffers();
    Free(gBagMenu);
    gBagMenu = NULL;
    ResetBgsAndClearDma3BusyFlags(FALSE);
}
