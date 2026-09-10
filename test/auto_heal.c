#include "global.h"
#include "auto_heal.h"
#include "battle.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "save.h"
#include "test/test.h"
#include "constants/flags.h"

TEST("Auto heal restores survivors and Orb charge without changing fainted Pokemon or items")
{
    struct Pokemon fainted;
    u32 value;

    ClearBag();
    AddBagItem(ITEM_TERA_ORB, 1);
    FlagSet(FLAG_RUN_RULE_AUTO_HEAL);
    FlagClear(FLAG_TERA_ORB_CHARGED);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    gBattleOutcome = B_OUTCOME_WON;
    gPartiesCount[B_TRAINER_PLAYER] = 2;
    for (u32 i = 0; i < 2; i++)
    {
        CreateRandomMon(&gParties[B_TRAINER_PLAYER][i], SPECIES_PIKACHU, 20);
        SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][i], MOVE_TACKLE, 0);
        value = STATUS1_BURN;
        SetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_STATUS, &value);
        value = 1 - i;
        SetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_HP, &value);
        value = 0;
        SetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_PP1, &value);
    }
    fainted = gParties[B_TRAINER_PLAYER][1];
    AutoHealAfterBattle();
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP), GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MAX_HP));
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_STATUS), 0);
    EXPECT(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_PP1) > 0);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM), ITEM_NONE);
    EXPECT_EQ(memcmp(&fainted, &gParties[B_TRAINER_PLAYER][1], sizeof(fainted)), 0);
    EXPECT(FlagGet(FLAG_TERA_ORB_CHARGED));
    FlagClear(FLAG_TERA_ORB_CHARGED);
    AutoHealAfterBattle();
    EXPECT(FlagGet(FLAG_TERA_ORB_CHARGED));
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    FlagClear(FLAG_TERA_ORB_CHARGED);
    FlagClear(FLAG_RUN_RULE_AUTO_HEAL);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT(FlagGet(FLAG_TERA_ORB_CHARGED));
    EXPECT(FlagGet(FLAG_RUN_RULE_AUTO_HEAL));
    FlagClear(FLAG_RUN_RULE_AUTO_HEAL);
    FlagClear(FLAG_TERA_ORB_CHARGED);
    ClearBag();
}

TEST("Auto heal respects its setting battle exclusions and defeat outcomes")
{
    u32 flags, outcome;
    bool32 enabled, expected;

    PARAMETRIZE { flags = 0; outcome = B_OUTCOME_CAUGHT; enabled = TRUE; expected = TRUE; }
    PARAMETRIZE { flags = 0; outcome = B_OUTCOME_RAN; enabled = TRUE; expected = TRUE; }
    PARAMETRIZE { flags = 0; outcome = B_OUTCOME_WON; enabled = FALSE; expected = FALSE; }
    PARAMETRIZE { flags = 0; outcome = B_OUTCOME_LOST; enabled = TRUE; expected = FALSE; }
    PARAMETRIZE { flags = 0; outcome = B_OUTCOME_DREW; enabled = TRUE; expected = FALSE; }
    PARAMETRIZE { flags = BATTLE_TYPE_LINK; outcome = B_OUTCOME_WON; enabled = TRUE; expected = FALSE; }
    PARAMETRIZE { flags = BATTLE_TYPE_RECORDED_LINK; outcome = B_OUTCOME_WON; enabled = TRUE; expected = FALSE; }
    PARAMETRIZE { flags = BATTLE_TYPE_FRONTIER; outcome = B_OUTCOME_WON; enabled = TRUE; expected = FALSE; }

    ClearBag();
    AddBagItem(ITEM_TERA_ORB, 1);
    FlagClear(FLAG_TERA_ORB_CHARGED);
    if (enabled)
        FlagSet(FLAG_RUN_RULE_AUTO_HEAL);
    else
        FlagClear(FLAG_RUN_RULE_AUTO_HEAL);
    gPartiesCount[B_TRAINER_PLAYER] = 0;
    gBattleTypeFlags = flags;
    gBattleOutcome = outcome;
    AutoHealAfterBattle();
    EXPECT_EQ(FlagGet(FLAG_TERA_ORB_CHARGED), expected);
    FlagClear(FLAG_RUN_RULE_AUTO_HEAL);
    FlagClear(FLAG_TERA_ORB_CHARGED);
    ClearBag();
}
