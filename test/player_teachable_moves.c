#include "global.h"
#include "event_data.h"
#include "player_teachable_moves.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/moves.h"
#include "constants/species.h"

static u32 GetNormalTeachableMoveCount(enum Species species)
{
    const u16 *learnset = GetSpeciesTeachableLearnset(species);
    u32 count = 0;

    while (learnset[count] != MOVE_UNAVAILABLE)
        count++;
    return count;
}

TEST("Full compatibility Off preserves species teachable moves")
{
    const u16 *learnset = GetSpeciesTeachableLearnset(SPECIES_BULBASAUR);
    u32 count = GetNormalTeachableMoveCount(SPECIES_BULBASAUR);

    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);

    EXPECT_EQ(GetPlayerTeachableMoveCount(SPECIES_BULBASAUR), count);
    for (u32 i = 0; i < count; i++)
        EXPECT_EQ(GetPlayerTeachableMove(SPECIES_BULBASAUR, i), learnset[i]);
    EXPECT_EQ(GetPlayerTeachableMove(SPECIES_BULBASAUR, count), MOVE_NONE);
    EXPECT_EQ(CanPlayerLearnTeachableMove(SPECIES_MAGIKARP, MOVE_FLY), CanLearnTeachableMove(SPECIES_MAGIKARP, MOVE_FLY));
}

TEST("Full compatibility On allows offered moves for valid non-Egg species")
{
    FlagSet(FLAG_RUN_RULE_FULL_COMPATIBILITY);

    EXPECT(!CanLearnTeachableMove(SPECIES_MAGIKARP, MOVE_FLY));
    EXPECT(CanPlayerLearnTeachableMove(SPECIES_MAGIKARP, MOVE_FLY));
    EXPECT(CanPlayerLearnTeachableMove(SPECIES_MAGIKARP, MOVE_BODY_SLAM));
    EXPECT(!CanPlayerLearnTeachableMove(SPECIES_MAGIKARP, MOVE_SPLASH));
    EXPECT(!CanPlayerLearnTeachableMove(SPECIES_EGG, MOVE_FLY));
    EXPECT(!CanPlayerLearnTeachableMove(SPECIES_NONE, MOVE_FLY));

    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
}

TEST("Full compatibility Pokédex moves match effective compatibility without duplicates")
{
    u32 count;

    FlagSet(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    count = GetPlayerTeachableMoveCount(SPECIES_BULBASAUR);

    EXPECT_GT(count, GetNormalTeachableMoveCount(SPECIES_BULBASAUR));
    for (u32 i = 0; i < count; i++)
    {
        enum Move move = GetPlayerTeachableMove(SPECIES_BULBASAUR, i);

        EXPECT_NE(move, MOVE_NONE);
        EXPECT(CanPlayerLearnTeachableMove(SPECIES_BULBASAUR, move));
        for (u32 j = 0; j < i; j++)
            EXPECT_NE(move, GetPlayerTeachableMove(SPECIES_BULBASAUR, j));
    }
    EXPECT_EQ(GetPlayerTeachableMove(SPECIES_BULBASAUR, count), MOVE_NONE);

    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
}
