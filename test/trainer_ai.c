#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "trainer_difficulty.h"
#include "trainer_randomizer.h"
#include "test/test.h"
#include "constants/battle_ai.h"
#include "constants/flags.h"
#include "constants/vars.h"

TEST("Trainer AI applies the exact difficulty and randomization flag matrix")
{
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    const u16 trainers[] = {TRAINER_CALVIN_1, TRAINER_ROXANNE_1, TRAINER_BRENDAN_ROUTE_103_TREECKO, TRAINER_WALLY_MAUVILLE, TRAINER_WALLY_VR_1};
    u64 authored = AI_FLAG_FORCE_SETUP_FIRST_TURN;
    for (u32 random = 0; random < 2; random++)
    {
        if (random)
            FlagSet(FLAG_RUN_RULE_TRAINERS);
        else
            FlagClear(FLAG_RUN_RULE_TRAINERS);
        for (u32 difficulty = 0; difficulty < 3; difficulty++)
        {
            VarSet(VAR_RUN_RULE_DIFFICULTY, difficulty);
            for (u32 i = 0; i < ARRAY_COUNT(trainers); i++)
            {
                bool32 boss = i == 1 || i == 4;
                u64 expected = random ? (boss ? AI_FLAG_BASIC_TRAINER : AI_FLAG_CHECK_BAD_MOVE) : authored;
                if (difficulty == 2 || (difficulty == 1 && boss))
                    expected |= AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES;
                if (difficulty == 2 && boss)
                    expected |= AI_FLAG_PP_STALL_PREVENTION;
                EXPECT_EQ(GetRunTrainerAIFlags(trainers[i], authored), expected);
            }
        }
    }
    gBattleTypeFlags |= BATTLE_TYPE_FRONTIER;
    EXPECT_EQ(GetRunTrainerAIFlags(TRAINER_ROXANNE_1, authored), authored);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    EXPECT_EQ(GetRunTrainerAIFlags(TRAINER_PARTNER(PARTNER_STEVEN), authored), authored);
}

u64 Test_GetTrainerAiFlags(u16 trainerId, enum BattlerId battler);

TEST("Trainer AI keeps double coordination and per-trainer difficulty profiles")
{
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    VarSet(VAR_RUN_RULE_DIFFICULTY, RUN_TRAINER_HARD);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TWO_OPPONENTS;
    gBattlersCount = 4;
    TRAINER_BATTLE_PARAM.opponentA = TRAINER_CALVIN_1;
    TRAINER_BATTLE_PARAM.opponentB = TRAINER_ROXANNE_1;
    EXPECT_EQ(Test_GetTrainerAiFlags(TRAINER_CALVIN_1, B_BATTLER_1), AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_DOUBLE_BATTLE);
    EXPECT_EQ(Test_GetTrainerAiFlags(TRAINER_ROXANNE_1, B_BATTLER_3),
        AI_FLAG_BASIC_TRAINER | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_DOUBLE_BATTLE);
}
