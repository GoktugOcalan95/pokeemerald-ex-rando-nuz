#include "global.h"
#include "event_data.h"
#include "field_move.h"
#include "item.h"
#include "save.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/field_move.h"

TEST("Early Surf replaces only Surf's badge requirement and persists")
{
    ClearBag();
    for (u32 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
        FlagClear(flag);
    FlagSet(FLAG_RUN_RULE_EARLY_SURF);
    FlagSet(FLAG_BADGE03_GET);
    EXPECT(!IsFieldMoveUnlocked(FIELD_MOVE_SURF));
    EXPECT(AddBagItem(ITEM_HM_SURF, 1));
    EXPECT(IsFieldMoveUnlocked(FIELD_MOVE_SURF));
    EXPECT(AddBagItem(ITEM_HM_DIVE, 1));
    EXPECT(!IsFieldMoveUnlocked(FIELD_MOVE_DIVE));
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    FlagClear(FLAG_RUN_RULE_EARLY_SURF);
    EXPECT(!IsFieldMoveUnlocked(FIELD_MOVE_SURF));
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT(IsFieldMoveUnlocked(FIELD_MOVE_SURF));
    FlagClear(FLAG_RUN_RULE_EARLY_SURF);
    FlagSet(FLAG_BADGE05_GET);
    EXPECT(IsFieldMoveUnlocked(FIELD_MOVE_SURF));
    FlagClear(FLAG_BADGE03_GET);
    FlagClear(FLAG_BADGE05_GET);
    ClearBag();
}
