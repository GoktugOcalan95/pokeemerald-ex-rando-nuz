#include "global.h"
#include "event_data.h"
#include "item.h"
#include "run_setup.h"
#include "save.h"
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

TEST("Reusable TMs run rule survives saving and loading")
{
    bool32 enabled;

    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }

    RunSetup_Begin();
    RunSetup_SetReusableTMs(enabled);
    RunSetup_EnterConfirmation();
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);

    if (enabled)
        FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);
    else
        FlagSet(FLAG_RUN_RULE_REUSABLE_TMS);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_REUSABLE_TMS), enabled);
    EXPECT_EQ(GetItemImportance(ITEM_TM01), enabled || I_REUSABLE_TMS);
    FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);
}
