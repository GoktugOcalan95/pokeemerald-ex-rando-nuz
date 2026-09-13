#include "global.h"
#include "registered_items.h"
#include "item.h"
#include "save.h"
#include "gpu_regs.h"
#include "sprite.h"
#include "test/test.h"

static void ResetRegistration(void)
{
    ClearBag();
    memset(gSaveBlock1Ptr->registeredItems, 0, sizeof(gSaveBlock1Ptr->registeredItems));
}

TEST("Registered items move between slots replace assignments and survive save load")
{
    static const u16 items[] = {ITEM_MACH_BIKE, ITEM_TOGGLE_REPEL, ITEM_OLD_ROD, ITEM_WAILMER_PAIL};
    ResetRegistration();
    for (u32 i = 0; i < ARRAY_COUNT(items); i++)
    {
        EXPECT(AddBagItem(items[i], 1));
        EXPECT(RegisterItem(i, items[i]));
        EXPECT_EQ(GetRegisteredItemSlot(items[i]), i);
    }
    EXPECT_EQ(ValidateRegisteredItems(), 4);
    EXPECT(RegisterItem(2, ITEM_MACH_BIKE));
    EXPECT_EQ(gSaveBlock1Ptr->registeredItems[0], ITEM_NONE);
    EXPECT_EQ(GetRegisteredItemSlot(ITEM_OLD_ROD), -1);
    EXPECT_EQ(GetRegisteredItemSlot(ITEM_MACH_BIKE), 2);
    EXPECT_EQ(ValidateRegisteredItems(), 3);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    memset(gSaveBlock1Ptr->registeredItems, 0, sizeof(gSaveBlock1Ptr->registeredItems));
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(GetRegisteredItemSlot(ITEM_TOGGLE_REPEL), 1);
    EXPECT_EQ(GetRegisteredItemSlot(ITEM_MACH_BIKE), 2);
    EXPECT_EQ(GetRegisteredItemSlot(ITEM_WAILMER_PAIL), 3);
    UnregisterItem(ITEM_TOGGLE_REPEL);
    EXPECT_EQ(GetRegisteredItemSlot(ITEM_TOGGLE_REPEL), -1);
    EXPECT(CheckBagHasItem(ITEM_TOGGLE_REPEL, 1));
    ResetRegistration();
}

TEST("Registered items clear unavailable invalid and duplicate slots before field dispatch")
{
    ResetRegistration();
    EXPECT(!RegisterItem(0, ITEM_OLD_ROD));
    EXPECT(AddBagItem(ITEM_OLD_ROD, 1));
    EXPECT(AddBagItem(ITEM_POTION, 1));
    EXPECT(!RegisterItem(0, ITEM_POTION));
    EXPECT(!RegisterItem(4, ITEM_OLD_ROD));
    EXPECT(!RegisterItem(0, ITEMS_COUNT));
    EXPECT(RegisterItem(0, ITEM_OLD_ROD));
    gSaveBlock1Ptr->registeredItems[1] = ITEM_OLD_ROD;
    gSaveBlock1Ptr->registeredItems[2] = ITEMS_COUNT;
    gSaveBlock1Ptr->registeredItems[3] = ITEM_POTION;
    EXPECT_EQ(ValidateRegisteredItems(), 1);
    EXPECT_EQ(gSaveBlock1Ptr->registeredItems[0], ITEM_OLD_ROD);
    for (u32 i = 1; i < REGISTERED_ITEMS_COUNT; i++)
        EXPECT_EQ(gSaveBlock1Ptr->registeredItems[i], ITEM_NONE);
    EXPECT(RemoveBagItem(ITEM_OLD_ROD, 1));
    EXPECT_EQ(ValidateRegisteredItems(), 0);
    ResetRegistration();
}

TEST("Registered items retain the bike direction through exchanges without changing other assignments")
{
    ResetRegistration();
    EXPECT(AddBagItem(ITEM_TOGGLE_REPEL, 1));
    EXPECT(RegisterItem(0, ITEM_TOGGLE_REPEL));
    EXPECT(AddBagItem(ITEM_MACH_BIKE, 1));
    SwapRegisteredBike();
    EXPECT_EQ(GetRegisteredItemSlot(ITEM_MACH_BIKE), -1);
    EXPECT(RegisterItem(3, ITEM_MACH_BIKE));
    SwapRegisteredBike();
    EXPECT_EQ(GetRegisteredItemSlot(ITEM_MACH_BIKE), 3);
    EXPECT(RemoveBagItem(ITEM_MACH_BIKE, 1));
    EXPECT(AddBagItem(ITEM_ACRO_BIKE, 1));
    SwapRegisteredBike();
    EXPECT_EQ(GetRegisteredItemSlot(ITEM_ACRO_BIKE), 3);
    EXPECT_EQ(GetRegisteredItemSlot(ITEM_MACH_BIKE), -1);
    EXPECT_EQ(GetRegisteredItemSlot(ITEM_TOGGLE_REPEL), 0);
    EXPECT(RemoveBagItem(ITEM_ACRO_BIKE, 1));
    EXPECT(AddBagItem(ITEM_MACH_BIKE, 1));
    SwapRegisteredBike();
    EXPECT_EQ(GetRegisteredItemSlot(ITEM_MACH_BIKE), 3);
    ResetRegistration();
}

TEST("Registered item wheel maps each direction to its saved slot")
{
    static const u16 keys[] = {DPAD_UP, DPAD_RIGHT, DPAD_DOWN, DPAD_LEFT};
    for (u32 i = 0; i < ARRAY_COUNT(keys); i++)
        EXPECT_EQ(RegisteredItemSlotFromKeys(keys[i]), i);
    EXPECT_EQ(RegisteredItemSlotFromKeys(0), -1);
    EXPECT_EQ(RegisteredItemSlotFromKeys(A_BUTTON | B_BUTTON | SELECT_BUTTON), -1);
}

TEST("Registered item wheel stays open on release and empty directions and closes on fresh cancel input")
{
    ResetRegistration();
    EXPECT(AddBagItem(ITEM_TOGGLE_REPEL, 1));
    EXPECT(RegisterItem(2, ITEM_TOGGLE_REPEL));
    EXPECT_EQ(RegisteredItemWheelInput(0), ITEM_NONE);
    EXPECT_EQ(RegisteredItemWheelInput(DPAD_UP), ITEM_NONE);
    EXPECT_EQ(RegisteredItemWheelInput(A_BUTTON), ITEM_NONE);
    EXPECT_EQ(RegisteredItemWheelInput(DPAD_DOWN), ITEM_TOGGLE_REPEL);
    EXPECT_EQ(RegisteredItemWheelInput(B_BUTTON), ITEMS_COUNT);
    EXPECT_EQ(RegisteredItemWheelInput(SELECT_BUTTON), ITEMS_COUNT);
    EXPECT_EQ(RegisteredItemWheelInput(B_BUTTON | DPAD_DOWN), ITEMS_COUNT);
    EXPECT_EQ(GetRegisteredItemSlot(ITEM_TOGGLE_REPEL), 2);
    ResetRegistration();
}

TEST("Registered item wheel releases graphics and restores field windows after repeated use")
{
    ResetRegistration();
    ResetSpriteData();
    FreeAllSpritePalettes();
    EXPECT(AddBagItem(ITEM_MACH_BIKE, 1));
    EXPECT(AddBagItem(ITEM_TOGGLE_REPEL, 1));
    EXPECT(RegisterItem(0, ITEM_MACH_BIKE));
    EXPECT(RegisterItem(1, ITEM_TOGGLE_REPEL));
    SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0);
    SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG_ALL | WININ_WIN0_OBJ);
    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_BG1 | BLDCNT_EFFECT_BLEND);
    SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(13, 7));
    ClearGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJWIN_ON);
    for (u32 iteration = 0; iteration < 20; iteration++)
    {
        EXPECT(ShowRegisteredItemWheel(iteration % 2));
        AnimateSprites();
        BuildOamBuffer();
        CloseRegisteredItemWheel();
        EXPECT_EQ(GetGpuReg(REG_OFFSET_WINOUT), WINOUT_WIN01_BG0);
        EXPECT_EQ(GetGpuReg(REG_OFFSET_WININ), WININ_WIN0_BG_ALL | WININ_WIN0_OBJ);
        EXPECT_EQ(GetGpuReg(REG_OFFSET_BLDCNT), BLDCNT_TGT2_BG1 | BLDCNT_EFFECT_BLEND);
        EXPECT_EQ(GetGpuReg(REG_OFFSET_BLDALPHA), BLDALPHA_BLEND(13, 7));
        EXPECT_EQ(GetGpuReg(REG_OFFSET_DISPCNT) & DISPCNT_OBJWIN_ON, 0);
        for (u32 i = 0; i < MAX_SPRITES; i++)
            EXPECT(!gSprites[i].inUse);
    }
}

TEST("Registered item wheel unwinds a partial open when sprites are exhausted")
{
    ResetRegistration();
    ResetSpriteData();
    FreeAllSpritePalettes();
    for (u32 i = 0; i < MAX_SPRITES - 1; i++)
        EXPECT(CreateSprite(&gDummySpriteTemplate, 0, 0, 0) != MAX_SPRITES);
    EXPECT(!ShowRegisteredItemWheel(FALSE));
    for (u32 i = 0; i < MAX_SPRITES - 1; i++)
        EXPECT(gSprites[i].inUse);
    EXPECT(!gSprites[MAX_SPRITES - 1].inUse);
    ResetSpriteData();
    EXPECT(ShowRegisteredItemWheel(FALSE));
    CloseRegisteredItemWheel();
}
