#include "global.h"
#include "event_data.h"
#include "field_move.h"
#include "item.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/field_move.h"

TEST("Early Fly requires HM02 without a badge")
{
    ClearBag();
    FlagClear(FLAG_BADGE06_GET);
    FlagSet(FLAG_RUN_RULE_EARLY_FLY);
    EXPECT(!IsFieldMoveUnlocked(FIELD_MOVE_FLY));
    EXPECT(AddBagItem(ITEM_HM_FLY, 1));
    EXPECT(IsFieldMoveUnlocked(FIELD_MOVE_FLY));
    FlagClear(FLAG_RUN_RULE_EARLY_FLY);
    EXPECT(!IsFieldMoveUnlocked(FIELD_MOVE_FLY));
    FlagSet(FLAG_BADGE06_GET);
    EXPECT(IsFieldMoveUnlocked(FIELD_MOVE_FLY));
    FlagClear(FLAG_BADGE06_GET);
    ClearBag();
}
