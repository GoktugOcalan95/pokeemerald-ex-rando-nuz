#ifndef GUARD_REGISTERED_ITEMS_H
#define GUARD_REGISTERED_ITEMS_H

s32 GetRegisteredItemSlot(u16 item);
void UnregisterItem(u16 item);
bool32 RegisterItem(u32 slot, u16 item);
u32 ValidateRegisteredItems(void);
s32 RegisteredItemSlotFromKeys(u16 keys);
u16 RegisteredItemWheelInput(u16 newKeys);
u8 ShowRegisteredItemWheel(bool32 inBag);
void CloseRegisteredItemWheel(u8 windowId, bool32 inBag);
extern const u8 *const gRegisteredItemDirections[REGISTERED_ITEMS_COUNT];

#endif
