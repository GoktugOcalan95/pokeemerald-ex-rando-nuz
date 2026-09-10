#ifndef GUARD_MACHRO_BIKE_H
#define GUARD_MACHRO_BIKE_H

bool32 IsBikeItem(u16 item);
u8 GetBikeItemMode(u16 item);
void UseBikeItem(u16 item);
bool32 CanSwitchMachroBike(void);
bool32 TrySwitchMachroBike(u16 newKeys, u16 heldKeys);
u32 GiveOrExchangeBike(u16 item);
void GiveOrExchangeBikeFromScript(void);

#endif
