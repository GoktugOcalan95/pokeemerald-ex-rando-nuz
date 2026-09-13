#include "global.h"
#include "battle.h"
#include "move_relearner.h"
#include "event_data.h"
#include "move.h"
#include "move_randomizer.h"
#include "pokemon.h"
#include "random.h"
#include "save.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/vars.h"

TEST("Randomized learnsets preserve schedules, uniqueness, power order and starting attacks for every species")
{
    u32 chance;
    PARAMETRIZE { chance = 0; }
    PARAMETRIZE { chance = 100; }
    gTestRunnerState.timeoutSeconds = 1800;
    FlagSet(FLAG_RUN_RULE_LEARNSETS);
    VarSet(VAR_RUN_RULE_GOOD_MOVE_CHANCE, chance);
    for (u32 species = 1; species < SPECIES_EGG; species++)
    {
        const struct LevelUpMove *original = gSpeciesInfo[species].levelUpLearnset;
        if (original == NULL)
            continue;
        const struct LevelUpMove *moves = GetSpeciesLevelUpLearnset(species);
        u8 counts[MAX_LEVEL + 1] = {0};
        bool8 used[MOVES_COUNT] = {0};
        u32 power = 0;
        bool32 attack = FALSE;
        for (u32 i = 0; moves[i].move != LEVEL_UP_MOVE_END; i++)
        {
            EXPECT_LT(i, 63);
            EXPECT(IsRandomizerMoveAllowed(moves[i].move));
            EXPECT(!used[moves[i].move]);
            used[moves[i].move] = TRUE;
            counts[moves[i].level]++;
            if (moves[i].level == 1)
                attack |= IsRandomizerDirectAttack(moves[i].move);
            if (moves[i].level && gMovesInfo[moves[i].move].category != DAMAGE_CATEGORY_STATUS)
            {
                EXPECT_GE(GetRandomizerMovePower(moves[i].move), power);
                power = GetRandomizerMovePower(moves[i].move);
            }
        }
        EXPECT(attack);
        EXPECT_EQ(counts[1], 4);
        for (u32 level = 0; level <= MAX_LEVEL; level++)
        {
            if (level == 1)
                continue;
            u32 expected = 0;
            bool32 occupied = FALSE;
            for (u32 i = 0; original[i].move != LEVEL_UP_MOVE_END; i++)
            {
                expected += original[i].level == level;
                occupied |= level >= 10 && original[i].level > level - 5 && original[i].level <= level;
            }
            if (level >= 10 && level <= 60 && level % 5 == 0 && !occupied)
                expected++;
            EXPECT_EQ(counts[level], expected);
        }
    }
}

TEST("Randomized learnsets agree with creation and learning and survive cache eviction and save load")
{
    struct Pokemon mon;
    struct LevelUpMove copy[64];
    FlagSet(FLAG_RUN_RULE_LEARNSETS);
    const struct LevelUpMove *moves = GetSpeciesLevelUpLearnset(SPECIES_SMEARGLE);
    u32 count = 0;
    do { copy[count] = moves[count]; } while (moves[count++].move != LEVEL_UP_MOVE_END);
    CreateMon(&mon, SPECIES_SMEARGLE, 1, 0, OTID_STRUCT_PLAYER_ID);
    GiveMonInitialMoveset(&mon);
    for (u32 i = 0; i < 4; i++)
        EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE1 + i), copy[i].move);
    CreateMon(&mon, SPECIES_SMEARGLE, 60, 0, OTID_STRUCT_PLAYER_ID);
    GiveMonInitialMoveset(&mon);
    u16 latest[4] = {0};
    for (u32 i = 0; copy[i].move != LEVEL_UP_MOVE_END && copy[i].level <= 60; i++)
    {
        for (u32 j = 0; j < 3; j++)
            latest[j] = latest[j + 1];
        latest[3] = copy[i].move;
    }
    for (u32 i = 0; i < 4; i++)
        EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE1 + i), latest[i]);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    gSaveBlock2Ptr->playerTrainerId[0] ^= 1;
    for (u32 species = SPECIES_BULBASAUR; species <= SPECIES_CHARIZARD; species++)
        GetSpeciesLevelUpLearnset(species);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    moves = GetSpeciesLevelUpLearnset(SPECIES_SMEARGLE);
    for (u32 i = 0; i < count; i++)
    {
        EXPECT_EQ(moves[i].move, copy[i].move);
        EXPECT_EQ(moves[i].level, copy[i].level);
    }
    FlagClear(FLAG_RUN_RULE_LEARNSETS);
    EXPECT_EQ(GetSpeciesLevelUpLearnset(SPECIES_SMEARGLE), gSpeciesInfo[SPECIES_SMEARGLE].levelUpLearnset);
}

TEST("Randomized learnsets offer matching padded and evolution moves")
{
    struct Pokemon mon;
    FlagSet(FLAG_RUN_RULE_LEARNSETS);
    CreateMon(&mon, SPECIES_SMEARGLE, 1, 0, OTID_STRUCT_PLAYER_ID);
    GiveMonInitialMoveset(&mon);
    const struct LevelUpMove *moves = GetSpeciesLevelUpLearnset(SPECIES_SMEARGLE);
    u32 index = 0;
    while (moves[index].level != 10)
        index++;
    EXPECT_EQ(MonTryLearningNewMoveAtLevel(&mon, TRUE, 10), MON_HAS_MAX_MOVES);
    EXPECT_EQ(gMoveToLearn, moves[index].move);
    EXPECT_EQ(MonTryLearningNewMoveAtLevel(&mon, FALSE, 10), MOVE_NONE);
    EXPECT(!HasMoveToRelearn(&mon.box, MOVE_RELEARNER_LEVEL_UP_MOVES));
    u16 empty = MOVE_NONE;
    SetMonData(&mon, MON_DATA_MOVE1, &empty);
    EXPECT(HasMoveToRelearn(&mon.box, MOVE_RELEARNER_LEVEL_UP_MOVES));
    CreateMon(&mon, SPECIES_RAICHU, 2, 0, OTID_STRUCT_PLAYER_ID);
    GiveMonInitialMoveset(&mon);
    moves = GetSpeciesLevelUpLearnset(SPECIES_RAICHU);
    bool32 first = TRUE;
    for (u32 i = 0; moves[i].move != LEVEL_UP_MOVE_END; i++)
        if (moves[i].level == 0)
        {
            EXPECT_EQ(MonTryLearningNewMoveEvolution(&mon, first), MON_HAS_MAX_MOVES);
            EXPECT_EQ(gMoveToLearn, moves[i].move);
            first = FALSE;
        }
    EXPECT(!first);
}
