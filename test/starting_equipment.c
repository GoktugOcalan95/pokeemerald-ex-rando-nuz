#include "global.h"
#include "config/overworld.h"
#include "event_data.h"
#include "item.h"
#include "item_use.h"
#include "new_game.h"
#include "registered_items.h"
#include "run_setup.h"
#include "save.h"
#include "test/test.h"
#include "constants/flags.h"

TEST("Starting equipment grants shoes and registered Machro independently of presets")
{
    u32 preset = 0;
    PARAMETRIZE { preset = RUN_SETUP_PRESET_VANILLA; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_NUZLOCKE; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_BISHEY; }
    RunSetup_Begin();
    RunSetup_SetPreset(preset);
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();
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
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    ClearBag();
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(CheckBagHasItem(ITEM_BICYCLE, 1), OW_START_WITH_SHOES_AND_MACHRO);
    EXPECT_EQ(FlagGet(FLAG_SYS_B_DASH), OW_START_WITH_SHOES_AND_MACHRO);
}
