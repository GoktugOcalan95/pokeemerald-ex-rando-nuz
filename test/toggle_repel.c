#include "global.h"
#include "event_data.h"
#include "item.h"
#include "item_use.h"
#include "save.h"
#include "test/test.h"
#include "constants/flags.h"

TEST("Toggle Repel starts enabled is registrable and shares the saved debug flag without consumption")
{
    ClearBag();
    gSaveBlock1Ptr->registeredItem = ITEM_NONE;
    FlagSet(FLAG_DEBUG_NO_ENCOUNTER);
    InitToggleRepel();
    EXPECT(!FlagGet(FLAG_DEBUG_NO_ENCOUNTER));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TOGGLE_REPEL), 1);
    EXPECT_EQ(gSaveBlock1Ptr->registeredItem, ITEM_NONE);
    EXPECT_EQ(GetItemType(ITEM_TOGGLE_REPEL), ITEM_USE_FIELD);
    EXPECT_EQ(GetItemFieldFunc(ITEM_TOGGLE_REPEL), ItemUseOutOfBattle_ToggleRepel);
    ToggleRepelEncounters();
    EXPECT(FlagGet(FLAG_DEBUG_NO_ENCOUNTER));
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    FlagClear(FLAG_DEBUG_NO_ENCOUNTER);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT(FlagGet(FLAG_DEBUG_NO_ENCOUNTER));
    ToggleRepelEncounters();
    EXPECT(!FlagGet(FLAG_DEBUG_NO_ENCOUNTER));
    FlagSet(FLAG_DEBUG_NO_ENCOUNTER);
    ToggleRepelEncounters();
    EXPECT(!FlagGet(FLAG_DEBUG_NO_ENCOUNTER));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TOGGLE_REPEL), 1);
    ClearBag();
}
