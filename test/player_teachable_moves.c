#include "global.h"
#include "event_data.h"
#include "item.h"
#include "teaching_randomizer.h"
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
    FlagSet(FLAG_RUN_RULE_EXPANDED_TMS);

    EXPECT_EQ(GetPlayerTeachableMoveCount(SPECIES_BULBASAUR), count);
    bool8 shown[MOVES_COUNT] = {0};
    for (u32 i = 0; i < count; i++)
        shown[GetPlayerTeachableMove(SPECIES_BULBASAUR, i)] = TRUE;
    for (u32 i = 0; i < count; i++)
        EXPECT(shown[learnset[i]]);
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
    bool8 seen[MOVES_COUNT] = {0};

    FlagSet(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    FlagSet(FLAG_RUN_RULE_EXPANDED_TMS);
    count = GetPlayerTeachableMoveCount(SPECIES_BULBASAUR);

    EXPECT_GT(count, GetNormalTeachableMoveCount(SPECIES_BULBASAUR));
    for (u32 i = 0; i < count; i++)
    {
        enum Move move = GetPlayerTeachableMove(SPECIES_BULBASAUR, i);

        EXPECT_NE(move, MOVE_NONE);
        EXPECT(CanPlayerLearnTeachableMove(SPECIES_BULBASAUR, move));
        EXPECT(!seen[move]);
        seen[move] = TRUE;
    }
    EXPECT_EQ(GetPlayerTeachableMove(SPECIES_BULBASAUR, count), MOVE_NONE);

    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
}

TEST("Teaching compatibility retains Baxcalibur native TM and inherited egg moves")
{
    static const u16 moves[] = {
        MOVE_SCARY_FACE, MOVE_THUNDER_FANG, MOVE_ZEN_HEADBUTT, MOVE_STOMPING_TANTRUM,
        MOVE_HELPING_HAND, MOVE_GIGA_IMPACT, MOVE_OUTRAGE, MOVE_DRACO_METEOR,
        MOVE_TERA_BLAST, MOVE_HIGH_HORSEPOWER, MOVE_ICICLE_SPEAR, MOVE_SCALE_SHOT,
        MOVE_DRAGON_CHEER, MOVE_AQUA_TAIL, MOVE_DRAGON_RUSH, MOVE_FREEZE_DRY,
    };
    bool8 found[ARRAY_COUNT(moves)] = {0};
    u32 remaining = ARRAY_COUNT(moves);
    bool32 tutors = FALSE;
    PARAMETRIZE { tutors = FALSE; }
    PARAMETRIZE { tutors = TRUE; }
    gTestRunnerState.timeoutSeconds = 180;
    InitEventData();
    for (u32 i = 0; i < ARRAY_COUNT(moves); i++)
    {
        EXPECT(!CanLearnTeachableMove(SPECIES_BAXCALIBUR, moves[i]));
        EXPECT(IsSpeciesCompatibleWithMove(SPECIES_BAXCALIBUR, moves[i]));
        EXPECT(!CanPlayerLearnTeachableMove(SPECIES_BAXCALIBUR, moves[i]));
    }
    FlagSet(FLAG_RUN_RULE_TMS_TUTORS);
    for (u32 seed = 0; seed < 256 && remaining; seed++)
    {
        gSaveBlock2Ptr->playerTrainerId[0] = seed;
        for (u32 slot = 0; (tutors ? GetTutorMove(slot) != MOVE_UNAVAILABLE : slot < GetActiveTMCount()); slot++)
        {
            u32 move = tutors ? GetTutorMove(slot) : GetTMHMMoveId(slot + 1);
            for (u32 i = 0; i < ARRAY_COUNT(moves); i++)
                if (!found[i] && move == moves[i])
                {
                    EXPECT(CanPlayerLearnTeachableMove(SPECIES_BAXCALIBUR, move));
                    found[i] = TRUE;
                    remaining--;
                }
        }
    }
    EXPECT_EQ(remaining, 0);
    InitEventData();
}

TEST("Teaching compatibility preserves species exceptions and validates move IDs")
{
    EXPECT(IsSpeciesCompatibleWithMove(SPECIES_MEW, MOVE_THUNDER_FANG));
    EXPECT(!IsSpeciesCompatibleWithMove(SPECIES_MEW, MOVE_DRAGON_ASCENT));
    EXPECT(IsSpeciesCompatibleWithMove(SPECIES_RAYQUAZA, MOVE_DRAGON_ASCENT));
    EXPECT(!IsSpeciesCompatibleWithMove(SPECIES_TERAPAGOS, MOVE_TERA_BLAST));
    EXPECT(!IsSpeciesCompatibleWithMove(SPECIES_MAGIKARP, MOVE_THUNDER_FANG));
    EXPECT(IsSpeciesCompatibleWithMove(SPECIES_MAGIKARP, MOVE_SPLASH));
    EXPECT(!IsSpeciesCompatibleWithMove(SPECIES_SMEARGLE, MOVE_THUNDER_FANG));
    EXPECT(!IsSpeciesCompatibleWithMove(SPECIES_NONE, MOVE_THUNDER_FANG));
    EXPECT(!IsSpeciesCompatibleWithMove(SPECIES_EGG, MOVE_THUNDER_FANG));
    EXPECT(!IsSpeciesCompatibleWithMove(NUM_SPECIES, MOVE_THUNDER_FANG));
    EXPECT(!IsSpeciesCompatibleWithMove(SPECIES_MEW, MOVE_NONE));
    EXPECT(!IsSpeciesCompatibleWithMove(SPECIES_MEW, MOVE_UNAVAILABLE));
    EXPECT(!IsSpeciesCompatibleWithMove(SPECIES_MEW, MOVE_STRUGGLE));
}

TEST("Teaching compatibility displays only active sources with either compatibility rule")
{
    bool32 full = FALSE;
    PARAMETRIZE { full = FALSE; }
    PARAMETRIZE { full = TRUE; }
    InitEventData();
    FlagSet(FLAG_RUN_RULE_TMS_TUTORS);
    if (full)
        FlagSet(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    bool8 offered[MOVES_COUNT] = {0};
    bool8 shown[MOVES_COUNT] = {0};
    for (u32 i = 1; i <= NUM_ALL_MACHINES; i++)
        offered[GetTMHMMoveId(i)] = TRUE;
    for (u32 i = 0; GetTutorMove(i) != MOVE_UNAVAILABLE; i++)
        offered[GetTutorMove(i)] = TRUE;
    u32 count = GetPlayerTeachableMoveCount(SPECIES_BAXCALIBUR);
    for (u32 i = 0; i < count; i++)
    {
        enum Move move = GetPlayerTeachableMove(SPECIES_BAXCALIBUR, i);
        EXPECT_NE(move, MOVE_NONE);
        EXPECT(offered[move]);
        EXPECT(!shown[move]);
        shown[move] = TRUE;
    }
    for (u32 move = 1; move < MOVES_COUNT; move++)
    {
        bool32 expected = offered[move] && (full || IsSpeciesCompatibleWithMove(SPECIES_BAXCALIBUR, move));
        EXPECT_EQ(CanPlayerLearnTeachableMove(SPECIES_BAXCALIBUR, move), expected);
        EXPECT_EQ(shown[move], expected);
    }
    InitEventData();
}
