#include "global.h"
#include "registered_items.h"
#include "bg.h"
#include "item.h"
#include "international_string_util.h"
#include "menu.h"
#include "string_util.h"
#include "text.h"
#include "window.h"

const u8 *const gRegisteredItemDirections[REGISTERED_ITEMS_COUNT] = {
    COMPOUND_STRING("UP"), COMPOUND_STRING("RIGHT"),
    COMPOUND_STRING("DOWN"), COMPOUND_STRING("LEFT"),
};

s32 GetRegisteredItemSlot(u16 item)
{
    if (item != ITEM_NONE)
        for (u32 i = 0; i < REGISTERED_ITEMS_COUNT; i++)
            if (gSaveBlock1Ptr->registeredItems[i] == item)
                return i;
    return -1;
}

void UnregisterItem(u16 item)
{
    for (u32 i = 0; i < REGISTERED_ITEMS_COUNT; i++)
        if (gSaveBlock1Ptr->registeredItems[i] == item)
            gSaveBlock1Ptr->registeredItems[i] = ITEM_NONE;
}

static bool32 CanRegisterItem(u16 item)
{
    return item > ITEM_NONE && item < ITEMS_COUNT
        && GetItemPocket(item) == POCKET_KEY_ITEMS
        && GetItemFieldFunc(item) != NULL
        && CheckBagHasItem(item, 1);
}

bool32 RegisterItem(u32 slot, u16 item)
{
    if (slot >= REGISTERED_ITEMS_COUNT || !CanRegisterItem(item))
        return FALSE;
    UnregisterItem(item);
    gSaveBlock1Ptr->registeredItems[slot] = item;
    return TRUE;
}

u32 ValidateRegisteredItems(void)
{
    u32 count = 0;
    for (u32 i = 0; i < REGISTERED_ITEMS_COUNT; i++)
    {
        u16 item = gSaveBlock1Ptr->registeredItems[i];
        if (!CanRegisterItem(item) || GetRegisteredItemSlot(item) != i)
            gSaveBlock1Ptr->registeredItems[i] = ITEM_NONE;
        else
            count++;
    }
    return count;
}

s32 RegisteredItemSlotFromKeys(u16 keys)
{
    if (keys & DPAD_UP)
        return 0;
    if (keys & DPAD_RIGHT)
        return 1;
    if (keys & DPAD_DOWN)
        return 2;
    if (keys & DPAD_LEFT)
        return 3;
    return -1;
}

u16 RegisteredItemWheelInput(u16 newKeys)
{
    s32 slot;
    if (newKeys & (B_BUTTON | SELECT_BUTTON))
        return ITEMS_COUNT;
    slot = RegisteredItemSlotFromKeys(newKeys);
    if (slot >= 0)
        return gSaveBlock1Ptr->registeredItems[slot];
    return ITEM_NONE;
}

u8 ShowRegisteredItemWheel(bool32 inBag)
{
    struct WindowTemplate template = {
        .tilemapLeft = 2, .tilemapTop = 3, .width = 26, .height = 13,
        .paletteNum = 15, .baseBlock = 0x280,
    };
    static const u8 positions[REGISTERED_ITEMS_COUNT][2] = {
        {56, 0}, {112, 32}, {56, 64}, {0, 32},
    };
    u8 name[ITEM_NAME_LENGTH + 7];
    u8 windowId;
    template.bg = inBag ? 1 : 0;
    windowId = AddWindow(&template);
    if (windowId == WINDOW_NONE)
        return windowId;
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    if (inBag)
        DrawStdFrameWithCustomTileAndPalette(windowId, FALSE, 1, 14);
    else
    {
        LoadMessageBoxAndBorderGfx();
        DrawStdWindowFrame(windowId, FALSE);
    }
    for (u32 i = 0; i < REGISTERED_ITEMS_COUNT; i++)
    {
        const u8 *label = gRegisteredItemDirections[i];
        u32 x = positions[i][0];
        u32 y = positions[i][1];
        u16 item = gSaveBlock1Ptr->registeredItems[i];
        AddTextPrinterParameterized(windowId, FONT_SMALL, label, x + GetStringCenterAlignXOffset(FONT_SMALL, label, 96), y, TEXT_SKIP_DRAW, NULL);
        if (item == ITEM_NONE)
            StringCopy(name, COMPOUND_STRING("--"));
        else
            CopyItemName(item, name);
        WrapFontIdToFit(name, name + StringLength(name), FONT_NARROW, 96);
        AddTextPrinterParameterized(windowId, FONT_NARROW, name, x + GetStringCenterAlignXOffset(FONT_NARROW, name, 96), y + 12, TEXT_SKIP_DRAW, NULL);
    }
    AddTextPrinterParameterized(windowId, FONT_SMALL, inBag ? COMPOUND_STRING("D-pad: Assign   B: Cancel") : COMPOUND_STRING("D-pad: Use   B/SELECT: Close"), 8, 90, TEXT_SKIP_DRAW, NULL);
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(template.bg);
    return windowId;
}

void CloseRegisteredItemWheel(u8 windowId, bool32 inBag)
{
    ClearStdWindowAndFrameToTransparent(windowId, FALSE);
    ClearWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_MAP);
    RemoveWindow(windowId);
    ScheduleBgCopyTilemapToVram(inBag ? 1 : 0);
}
