#include "global.h"
#include "caps.h"
#include "daycare.h"
#include "event_data.h"
#include "pokemon.h"
#include "run_setup.h"
#include "save.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/items.h"

static void ResetLevelCapProgress(void)
{
    for (u32 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
        FlagClear(flag);
    FlagClear(FLAG_IS_CHAMPION);
}

TEST("Level caps follow badges, stay at 58 through the League, and lift after Champion")
{
    static const u8 caps[] = {15, 19, 24, 29, 31, 33, 42, 46, 58};

    ResetLevelCapProgress();
    for (u32 badges = 0; badges < ARRAY_COUNT(caps); badges++)
    {
        FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
        EXPECT_EQ(GetCurrentLevelCap(), MAX_LEVEL);
        FlagSet(FLAG_RUN_RULE_LEVEL_CAPS);
        EXPECT_EQ(GetCurrentLevelCap(), caps[badges]);
        if (badges < 8)
            FlagSet(FLAG_BADGE01_GET + badges);
    }
    FlagSet(FLAG_IS_CHAMPION);
    EXPECT_EQ(GetCurrentLevelCap(), MAX_LEVEL);
    FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
    ResetLevelCapProgress();
}

TEST("Level caps run rule and progression survive saving and loading")
{
    bool32 enabled = FALSE;

    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }

    ResetLevelCapProgress();
    RunSetup_Begin();
    RunSetup_SetLevelCaps(enabled);
    RunSetup_EnterConfirmation();
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();
    FlagSet(FLAG_BADGE01_GET);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    FlagClear(FLAG_BADGE01_GET);
    if (enabled)
        FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
    else
        FlagSet(FLAG_RUN_RULE_LEVEL_CAPS);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_LEVEL_CAPS), enabled);
    EXPECT_EQ(GetCurrentLevelCap(), enabled ? 19 : MAX_LEVEL);
    FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
    ResetLevelCapProgress();
}

TEST("Level caps clamp candies and reject them at or above cap without altering the Pokemon")
{
    struct Pokemon mon;
    bool32 enabled = FALSE;
    u32 level = 14;
    enum Item item = ITEM_NONE;
    u32 initialExp;
    u32 friendship;

    for (u32 rule = FALSE; rule <= TRUE; rule++)
    {
        for (u32 initialLevel = 14; initialLevel <= 16; initialLevel++)
        {
            PARAMETRIZE { enabled = rule; level = initialLevel; item = ITEM_RARE_CANDY; }
            for (enum Item candy = ITEM_EXP_CANDY_XS; candy <= ITEM_EXP_CANDY_XL; candy++)
                PARAMETRIZE { enabled = rule; level = initialLevel; item = candy; }
        }
    }

    ResetLevelCapProgress();
    if (enabled)
        FlagSet(FLAG_RUN_RULE_LEVEL_CAPS);
    else
        FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
    CreateRandomMonWithIVs(&mon, SPECIES_WOBBUFFET, level, 0);
    initialExp = GetMonData(&mon, MON_DATA_EXP);
    friendship = GetMonData(&mon, MON_DATA_FRIENDSHIP);
    EXPECT_EQ(ExecuteTableBasedItemEffect(&mon, item, 0, 0), enabled && level >= 15);
    if (enabled && level >= 15)
    {
        EXPECT_EQ(GetMonData(&mon, MON_DATA_EXP), initialExp);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_LEVEL), level);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_FRIENDSHIP), friendship);
    }
    else
    {
        EXPECT_GT(GetMonData(&mon, MON_DATA_EXP), initialExp);
        if (enabled)
            EXPECT_LE(GetMonData(&mon, MON_DATA_EXP), gExperienceTables[gSpeciesInfo[SPECIES_WOBBUFFET].growthRate][15]);
    }
    FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
}

TEST("Level caps limit Day Care gains and preserve overleveled Pokemon and withdrawal prices")
{
    struct Pokemon mon;
    bool32 enabled = FALSE;
    u32 level = 14;
    u32 expectedLevel;

    for (u32 rule = FALSE; rule <= TRUE; rule++)
    {
        PARAMETRIZE { enabled = rule; level = 14; }
        PARAMETRIZE { enabled = rule; level = 15; }
        PARAMETRIZE { enabled = rule; level = 20; }
    }

    ResetLevelCapProgress();
    if (enabled)
        FlagSet(FLAG_RUN_RULE_LEVEL_CAPS);
    else
        FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
    ZeroPlayerPartyMons();
    memset(&gSaveBlock1Ptr->daycare, 0, sizeof(gSaveBlock1Ptr->daycare));
    CreateRandomMonWithIVs(&mon, SPECIES_WOBBUFFET, level, 0);
    gSaveBlock1Ptr->daycare.mons[0].mon = mon.box;
    gSaveBlock1Ptr->daycare.mons[0].steps = gExperienceTables[gSpeciesInfo[SPECIES_WOBBUFFET].growthRate][30] - GetMonData(&mon, MON_DATA_EXP);
    gSpecialVar_0x8004 = 0;
    expectedLevel = enabled ? (level < 15 ? 15 : level) : 30;
    EXPECT_EQ(GetNumLevelsGainedFromDaycare(), expectedLevel - level);
    GetDaycareCostAndPrepareString();
    EXPECT_EQ(gSpecialVar_0x8005, 100 + 100 * (expectedLevel - level));
    TakePokemonFromDaycare();
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_LEVEL), expectedLevel);
    FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
}
