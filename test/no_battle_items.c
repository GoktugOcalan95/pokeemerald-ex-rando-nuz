#include "global.h"
#include "battle.h"
#include "battle_ai_items.h"
#include "event_data.h"
#include "item.h"
#include "item_use.h"
#include "run_setup.h"
#include "save.h"
#include "test/test.h"
#include "constants/flags.h"

TEST("No Battle Items permits only balls and escape items while preserving the Bag")
{
    bool32 enabled = FALSE;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    RunSetup_Begin();
    RunSetup_SetValue(RUN_SETUP_NO_BATTLE_ITEMS, enabled);
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();
    const u16 blocked[] = {ITEM_POTION, ITEM_FULL_RESTORE, ITEM_REVIVE, ITEM_ETHER, ITEM_X_ATTACK, ITEM_ORAN_BERRY, ITEM_LUM_BERRY};
    const u16 allowed[] = {ITEM_POKE_BALL, ITEM_ULTRA_BALL, ITEM_POKE_DOLL, ITEM_FLUFFY_TAIL, ITEM_POKE_TOY};
    for (u32 i = 0; i < ARRAY_COUNT(blocked); i++)
        EXPECT_EQ(IsBattleItemBlockedByRunRule(blocked[i]), enabled);
    for (u32 i = 0; i < ARRAY_COUNT(allowed); i++)
        EXPECT(!IsBattleItemBlockedByRunRule(allowed[i]));
    if (enabled)
        EXPECT(!ShouldUseItem((enum BattlerId)1));
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    FlagClear(FLAG_RUN_RULE_NO_BATTLE_ITEMS);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_NO_BATTLE_ITEMS), enabled);
}
