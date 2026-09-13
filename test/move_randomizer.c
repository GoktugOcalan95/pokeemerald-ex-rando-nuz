#include "global.h"
#include "event_data.h"
#include "move.h"
#include "move_randomizer.h"
#include "random.h"
#include "save.h"
#include "teaching_randomizer.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/vars.h"

TEST("Good move chance classifies ordinary attacks and excludes prerequisites")
{
    const u16 good[] = {MOVE_SWIFT, MOVE_SURF, MOVE_DOUBLE_EDGE, MOVE_CLOSE_COMBAT, MOVE_BULLET_SEED, MOVE_DOUBLE_HIT};
    const u16 other[] = {MOVE_TACKLE, MOVE_THUNDER, MOVE_HYPER_BEAM, MOVE_SOLAR_BEAM, MOVE_EXPLOSION,
        MOVE_COUNTER, MOVE_SEISMIC_TOSS, MOVE_FISSURE, MOVE_DREAM_EATER, MOVE_SNORE, MOVE_BELCH,
        MOVE_SUCKER_PUNCH, MOVE_FUTURE_SIGHT, MOVE_CRUSH_GRIP, MOVE_TRIPLE_KICK, MOVE_STORED_POWER, MOVE_GYRO_BALL, MOVE_FLING, MOVE_LAST_RESORT};
    for (u32 i = 0; i < ARRAY_COUNT(good); i++)
        EXPECT(IsRandomizerGoodAttack(good[i]));
    for (u32 i = 0; i < ARRAY_COUNT(other); i++)
        EXPECT(!IsRandomizerGoodAttack(other[i]));
    EXPECT(!IsRandomizerMoveAllowed(MOVE_STRUGGLE));
    EXPECT(!IsRandomizerMoveAllowed(FIRST_Z_MOVE));
    EXPECT(!IsRandomizerMoveAllowed(FIRST_MAX_MOVE));
    EXPECT_EQ(GetRandomizerMovePower(MOVE_HYPER_BEAM), 67);
    EXPECT_EQ(GetRandomizerMovePower(MOVE_BULLET_SEED), 77);
    EXPECT_EQ(GetRandomizerMovePower(MOVE_SWORDS_DANCE), 0);
    EXPECT_EQ(GetRandomizerMovePower(MOVE_BIDE), 16);
    EXPECT_EQ(GetRandomizerMovePower(MOVE_HARD_PRESS), 100);
    EXPECT_EQ(GetRandomizerMovePower(MOVE_ELECTRO_BALL), 60);
    EXPECT_EQ(GetRandomizerMovePower(MOVE_GYRO_BALL), 26);
}

TEST("Good move chance biases unique teaching moves and restores across save load")
{
    bool8 used[MOVES_COUNT] = {0};
    u16 moves[80];
    FlagSet(FLAG_RUN_RULE_TMS_TUTORS);
    VarSet(VAR_RUN_RULE_GOOD_MOVE_CHANCE, 100);
    SeedRng(123);
    u32 next = Random();
    SeedRng(123);
    for (u32 i = 0; i < ARRAY_COUNT(moves); i++)
    {
        moves[i] = i < 50 ? GetRandomizedMachineMove(i + 1) : GetTutorMove(i - 50);
        EXPECT(IsRandomizerGoodAttack(moves[i]));
        EXPECT(!used[moves[i]]);
        used[moves[i]] = TRUE;
    }
    EXPECT_EQ(Random(), next);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    VarSet(VAR_RUN_RULE_GOOD_MOVE_CHANCE, 0);
    bool32 hasOther = FALSE;
    for (u32 i = 0; i < 50; i++)
        hasOther |= !IsRandomizerGoodAttack(GetRandomizedMachineMove(i + 1));
    EXPECT(hasOther);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    for (u32 i = 0; i < ARRAY_COUNT(moves); i++)
        EXPECT_EQ(i < 50 ? GetRandomizedMachineMove(i + 1) : GetTutorMove(i - 50), moves[i]);
    for (u32 move = 1; move < MOVES_COUNT; move++)
        used[move] = IsRandomizerGoodAttack(move);
    EXPECT_NE(ChooseRandomizerMove(123, 0, 0, used), MOVE_NONE);
}
