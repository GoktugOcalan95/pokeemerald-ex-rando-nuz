#include "global.h"
#include "event_data.h"
#include "pokemon.h"
#include "random.h"
#include "wild_encounter.h"
#include "test/test.h"
#include "constants/flags.h"

extern void RockSmashWildEncounter(void);

TEST("Rock Smash Encounter OFF returns without RNG or Pokemon generation")
{
    struct Pokemon opponent;
    rng_value_t seed;

    CreateRandomMon(&gParties[B_TRAINER_OPPONENT_A][0], SPECIES_GEODUDE, 20);
    opponent = gParties[B_TRAINER_OPPONENT_A][0];
    seed = gRngValue;
    FlagSet(FLAG_DEBUG_NO_ENCOUNTER);
    gSpecialVar_Result = TRUE;
    RockSmashWildEncounter();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(memcmp(&gRngValue, &seed, sizeof(seed)), 0);
    EXPECT_EQ(memcmp(&opponent, &gParties[B_TRAINER_OPPONENT_A][0], sizeof(opponent)), 0);
    FlagClear(FLAG_DEBUG_NO_ENCOUNTER);
}
