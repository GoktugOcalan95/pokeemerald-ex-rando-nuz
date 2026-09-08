#include "global.h"
#include "caps.h"
#include "event_data.h"
#include "pokemon.h"
#include "party_menu.h"
#include "save.h"
#include "test/test.h"
#include "constants/flags.h"

static void ResetCapProgress(void)
{
    for (u32 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
        FlagClear(flag);
    FlagClear(FLAG_IS_CHAMPION);
}

TEST("Level to cap uses exact EXP thresholds for all growth rates and every cap")
{
    static const enum Species species[] = {SPECIES_NINCADA, SPECIES_SHROOMISH, SPECIES_BULBASAUR, SPECIES_CLEFAIRY, SPECIES_MAGIKARP, SPECIES_WOBBUFFET};
    static const u32 caps[] = {15, 19, 24, 29, 31, 33, 42, 46, 58, 100};
    struct Pokemon mon;
    u32 rates = 0;

    FlagSet(FLAG_RUN_RULE_LEVEL_CAPS);
    for (u32 i = 0; i < ARRAY_COUNT(species); i++)
    {
        rates |= 1u << gSpeciesInfo[species[i]].growthRate;
        ResetCapProgress();
        for (u32 milestone = 0; milestone < ARRAY_COUNT(caps); milestone++)
        {
            CreateRandomMon(&mon, species[i], 5);
            u32 personality = GetMonData(&mon, MON_DATA_PERSONALITY);
            EXPECT(RaiseMonToLevelCap(&mon));
            EXPECT_EQ(GetMonData(&mon, MON_DATA_LEVEL), caps[milestone]);
            EXPECT_EQ(GetMonData(&mon, MON_DATA_EXP), gExperienceTables[gSpeciesInfo[species[i]].growthRate][caps[milestone]]);
            EXPECT_EQ(GetMonData(&mon, MON_DATA_SPECIES), species[i]);
            EXPECT_EQ(GetMonData(&mon, MON_DATA_PERSONALITY), personality);
            if (milestone < 8)
                FlagSet(FLAG_BADGE01_GET + milestone);
            else
                FlagSet(FLAG_IS_CHAMPION);
        }
    }
    EXPECT_EQ(rates, (1u << 6) - 1);
    ResetCapProgress();
    FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
}

TEST("Level to cap leaves Eggs, capped and overleveled Pokemon, and disabled runs unchanged")
{
    struct Pokemon mon, original;
    u32 level = 0;
    bool32 egg = FALSE;
    bool32 enabled = FALSE;
    PARAMETRIZE { level = 5; egg = TRUE; enabled = TRUE; }
    PARAMETRIZE { level = 15; egg = FALSE; enabled = TRUE; }
    PARAMETRIZE { level = 20; egg = FALSE; enabled = TRUE; }
    PARAMETRIZE { level = 5; egg = FALSE; enabled = FALSE; }
    ResetCapProgress();
    if (enabled)
        FlagSet(FLAG_RUN_RULE_LEVEL_CAPS);
    else
        FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
    CreateRandomMon(&mon, SPECIES_TREECKO, level);
    SetMonData(&mon, MON_DATA_IS_EGG, &egg);
    original = mon;
    EXPECT(!RaiseMonToLevelCap(&mon));
    EXPECT_EQ(memcmp(&mon, &original, sizeof(mon)), 0);
    FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
}

TEST("Level to cap EXP and stats persist through saving and loading")
{
    struct Pokemon expected;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    ResetCapProgress();
    FlagSet(FLAG_RUN_RULE_LEVEL_CAPS);
    CreateRandomMon(mon, SPECIES_TREECKO, 5);
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    EXPECT(RaiseMonToLevelCap(mon));
    expected = *mon;
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    ZeroMonData(mon);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(GetMonData(mon, MON_DATA_EXP), GetMonData(&expected, MON_DATA_EXP));
    EXPECT_EQ(GetMonData(mon, MON_DATA_LEVEL), GetMonData(&expected, MON_DATA_LEVEL));
    EXPECT_EQ(GetMonData(mon, MON_DATA_ATK), GetMonData(&expected, MON_DATA_ATK));
    FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
}

TEST("Level to cap continues learning after any learned move")
{
    EXPECT(Test_PartyMenuContinuesLevelUpLearning(MOVE_POUND));
    EXPECT(Test_PartyMenuContinuesLevelUpLearning(MOVE_LEER));
    EXPECT(Test_PartyMenuContinuesLevelUpLearning(MOVE_AGILITY));
    EXPECT(Test_PartyMenuContinuesLevelUpLearning(MOVE_TIDY_UP));
}
