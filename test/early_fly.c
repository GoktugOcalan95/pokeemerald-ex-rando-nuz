#include "global.h"
#include "event_data.h"
#include "field_move.h"
#include "item.h"
#include "save.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/field_move.h"

extern bool32 Test_StartMenuHasFly(u32 *count);

TEST("Early Fly requires HM02 without a badge and updates the Start menu after save load")
{
    u32 count;

    ClearBag();
    FlagClear(FLAG_BADGE06_GET);
    FlagSet(FLAG_RUN_RULE_EARLY_FLY);
    EXPECT(!IsFieldMoveUnlocked(FIELD_MOVE_FLY));
    EXPECT(!Test_StartMenuHasFly(&count));
    EXPECT(AddBagItem(ITEM_HM_FLY, 1));
    EXPECT(IsFieldMoveUnlocked(FIELD_MOVE_FLY));
    EXPECT(Test_StartMenuHasFly(&count));
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    FlagClear(FLAG_RUN_RULE_EARLY_FLY);
    EXPECT(!IsFieldMoveUnlocked(FIELD_MOVE_FLY));
    EXPECT(!Test_StartMenuHasFly(&count));
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT(IsFieldMoveUnlocked(FIELD_MOVE_FLY));
    EXPECT(Test_StartMenuHasFly(&count));
    FlagClear(FLAG_RUN_RULE_EARLY_FLY);
    FlagSet(FLAG_BADGE06_GET);
    EXPECT(IsFieldMoveUnlocked(FIELD_MOVE_FLY));
    FlagClear(FLAG_BADGE06_GET);
    ClearBag();
}
