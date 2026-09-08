#include "global.h"
#include "event_data.h"
#include "battle.h"
#include "battle_interface.h"
#include "run_setup.h"
#include "save.h"
#include "test/test.h"
#include "constants/flags.h"

TEST("Opponent HP percentage run rule survives saving and loading")
{
    bool32 enabled;

    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }

    RunSetup_Begin();
    RunSetup_SetOpponentHPPercentage(enabled);
    RunSetup_EnterConfirmation();
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);

    if (enabled)
        FlagClear(FLAG_RUN_RULE_OPPONENT_HP_PERCENTAGE);
    else
        FlagSet(FLAG_RUN_RULE_OPPONENT_HP_PERCENTAGE);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_OPPONENT_HP_PERCENTAGE), enabled);
    EXPECT_EQ(ShouldDisplayOpponentHPPercentage(), enabled);
    FlagClear(FLAG_RUN_RULE_OPPONENT_HP_PERCENTAGE);
}

TEST("Opponent HP percentage shifts only the singles opponent healthbox")
{
    bool32 enabled;
    bool32 doubles;
    s16 x, y;
    u32 previousBattleType = gBattleTypeFlags;
    u8 previousPlayerPosition = gBattlerPositions[0];
    u8 previousOpponentPosition = gBattlerPositions[1];

    PARAMETRIZE { enabled = FALSE; doubles = FALSE; }
    PARAMETRIZE { enabled = TRUE; doubles = FALSE; }
    PARAMETRIZE { enabled = FALSE; doubles = TRUE; }
    PARAMETRIZE { enabled = TRUE; doubles = TRUE; }

    if (enabled)
        FlagSet(FLAG_RUN_RULE_OPPONENT_HP_PERCENTAGE);
    else
        FlagClear(FLAG_RUN_RULE_OPPONENT_HP_PERCENTAGE);
    gBattleTypeFlags = doubles ? BATTLE_TYPE_DOUBLE | BATTLE_TYPE_MULTI | BATTLE_TYPE_TWO_OPPONENTS : 0;
    gBattlerPositions[0] = B_POSITION_PLAYER_LEFT;
    gBattlerPositions[1] = B_POSITION_OPPONENT_LEFT;
    GetBattlerHealthboxCoords(0, &x, &y);
    EXPECT_EQ(x, doubles ? 159 : 158);
    EXPECT_EQ(y, doubles ? 76 : 88);
    GetBattlerHealthboxCoords(1, &x, &y);
    EXPECT_EQ(x, 44);
    EXPECT_EQ(y, doubles ? 19 : enabled ? 22 : 30);

    gBattleTypeFlags = previousBattleType;
    gBattlerPositions[0] = previousPlayerPosition;
    gBattlerPositions[1] = previousOpponentPosition;
    FlagClear(FLAG_RUN_RULE_OPPONENT_HP_PERCENTAGE);
}
