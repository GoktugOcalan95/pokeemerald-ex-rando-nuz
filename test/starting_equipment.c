#include "global.h"
#include "config/overworld.h"
#include "event_data.h"
#include "item.h"
#include "item_use.h"
#include "new_game.h"
#include "registered_items.h"
#include "test/test.h"
#include "constants/flags.h"

TEST("Starting equipment grants shoes and registered Machro")
{
    ClearBag();
    InitToggleRepel();
    InitStartingEquipment();
    EXPECT_EQ(CheckBagHasItem(ITEM_BICYCLE, 1), OW_START_WITH_SHOES_AND_MACHRO);
    EXPECT(!CheckBagHasItem(ITEM_BICYCLE, 2));
    EXPECT_EQ(FlagGet(FLAG_RECEIVED_BIKE), OW_START_WITH_SHOES_AND_MACHRO);
    EXPECT_EQ(FlagGet(FLAG_RECEIVED_RUNNING_SHOES), OW_START_WITH_SHOES_AND_MACHRO);
    EXPECT_EQ(FlagGet(FLAG_SYS_B_DASH), OW_START_WITH_SHOES_AND_MACHRO);
    EXPECT(!FlagGet(FLAG_MACHRO_ACRO_MODE));
    EXPECT(!FlagGet(FLAG_DEBUG_NO_ENCOUNTER));
    if (OW_START_WITH_SHOES_AND_MACHRO)
    {
        EXPECT_EQ(RegisteredItemWheelInput(DPAD_RIGHT), ITEM_BICYCLE);
        EXPECT_EQ(RegisteredItemWheelInput(DPAD_DOWN), ITEM_TOGGLE_REPEL);
        EXPECT_EQ(RegisteredItemWheelInput(DPAD_UP), ITEM_NONE);
        EXPECT_EQ(RegisteredItemWheelInput(DPAD_LEFT), ITEM_NONE);
    }
}
