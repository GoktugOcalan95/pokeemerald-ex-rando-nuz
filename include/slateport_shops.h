#ifndef GUARD_SLATEPORT_SHOPS_H
#define GUARD_SLATEPORT_SHOPS_H

#include "constants/slateport_shops.h"

u32 BuildSlateportShopStock(u32 category, u16 *items);
bool32 IsSlateportPreChampionItem(u16 item);
bool32 IsFunctionalEvolutionOrFormItem(u16 item);
u32 GetSlateportItemPrice(u16 item, bool32 discount);
u32 LimitSlateportResalePrice(u16 item, u32 price);
bool32 TryGiveSlateportPurchase(u32 category, u16 item, u16 count);
void OpenSlateportShop(void);
void CheckSlateportSpecialStock(void);

#endif
