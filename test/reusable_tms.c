#include "global.h"
#include "event_data.h"
#include "item.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/items.h"

TEST("Reusable TMs preserves every TM through the teaching importance check")
{
    FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);
    for (enum Item item = ITEM_TM01; item < ITEM_HM01; item++)
        EXPECT_EQ(GetItemImportance(item), I_REUSABLE_TMS);

    FlagSet(FLAG_RUN_RULE_REUSABLE_TMS);
    for (enum Item item = ITEM_TM01; item < ITEM_HM01; item++)
        EXPECT(GetItemImportance(item));

    EXPECT(GetItemImportance(ITEM_HM01));
    EXPECT(GetItemImportance(ITEM_HM08));
    EXPECT(!GetItemImportance(ITEM_POTION));
    EXPECT(!GetItemImportance(ITEM_NONE));
    FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);
    EXPECT(GetItemImportance(ITEM_HM01));
}

