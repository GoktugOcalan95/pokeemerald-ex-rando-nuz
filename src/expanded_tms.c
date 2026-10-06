#include "global.h"
#include "coins.h"
#include "event_data.h"
#include "expanded_tms.h"
#include "item.h"
#include "teaching_randomizer.h"
#include "constants/vars.h"

// The expanded counterpart of TMxx is TMxx + 50, found by item ID offset.
STATIC_ASSERT(ITEM_TM100 == ITEM_TM01 + 99, TMItemIdsContiguous);

static const u16 sGameCornerTMs[] = {ITEM_TM32, ITEM_TM29, ITEM_TM35, ITEM_TM24, ITEM_TM13};

u16 GetGameCornerTM(u32 selection, bool32 expanded)
{
    if (selection >= ARRAY_COUNT(sGameCornerTMs) || (expanded && !IsExpandedTMListEnabled()))
        return ITEM_NONE;
    return sGameCornerTMs[selection] + (expanded ? 50 : 0);
}

u32 BuyGameCornerTM(u16 item)
{
    bool32 offered = FALSE;
    for (u32 i = 0; i < ARRAY_COUNT(sGameCornerTMs); i++)
        offered |= item == GetGameCornerTM(i, FALSE) || item == GetGameCornerTM(i, TRUE);
    if (item == ITEM_NONE || !offered)
        return 0;
    if (GetItemImportance(item) && (CheckBagHasItem(item, 1) || CheckPCHasItem(item, 1)))
        return 3;
    if (GetCoins() < 50)
        return 2;
    if (!AddBagItem(item, 1))
        return 0;
    RemoveCoins(50);
    return 1;
}

void SelectGameCornerTMFromScript(void)
{
    gSpecialVar_0x8004 = GetGameCornerTM(gSpecialVar_Result, VarGet(VAR_TEMP_3));
}

void BuyGameCornerTMFromScript(void)
{
    gSpecialVar_Result = BuyGameCornerTM(gSpecialVar_0x8004);
}
