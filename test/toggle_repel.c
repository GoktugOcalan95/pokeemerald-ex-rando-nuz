#include "global.h"
#include "event_data.h"
#include "item.h"
#include "item_use.h"
#include "test/test.h"
#include "constants/flags.h"

TEST("Toggle Repel starts enabled and toggles the debug encounter flag without consumption")
{
    ClearBag();
    memset(gSaveBlock1Ptr->registeredItems, 0, sizeof(gSaveBlock1Ptr->registeredItems));
    FlagSet(FLAG_DEBUG_NO_ENCOUNTER);
    InitToggleRepel();
    EXPECT(!FlagGet(FLAG_DEBUG_NO_ENCOUNTER));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TOGGLE_REPEL), 1);
    for (u32 i = 0; i < REGISTERED_ITEMS_COUNT; i++)
        EXPECT_EQ(gSaveBlock1Ptr->registeredItems[i], ITEM_NONE);
    ToggleRepelEncounters();
    EXPECT(FlagGet(FLAG_DEBUG_NO_ENCOUNTER));
    ToggleRepelEncounters();
    EXPECT(!FlagGet(FLAG_DEBUG_NO_ENCOUNTER));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TOGGLE_REPEL), 1);
    ClearBag();
}
