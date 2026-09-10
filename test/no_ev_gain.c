#include "global.h"
#include "event_data.h"
#include "pokemon.h"
#include "run_setup.h"
#include "save.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/items.h"

TEST("No EV gain preserves existing EVs and blocks battle training bonuses")
{
    struct Pokemon mon;
    bool32 enabled = FALSE;
    enum Item item = ITEM_NONE;
    u8 pokerus = 0x11;
    u8 ev = 10;

    for (u32 rule = FALSE; rule <= TRUE; rule++)
    {
        PARAMETRIZE { enabled = rule; item = ITEM_NONE; }
        PARAMETRIZE { enabled = rule; item = ITEM_MACHO_BRACE; }
        PARAMETRIZE { enabled = rule; item = ITEM_POWER_BRACER; }
    }

    CreateRandomMonWithIVs(&mon, SPECIES_WOBBUFFET, 50, 0);
    SetMonData(&mon, MON_DATA_HELD_ITEM, &item);
    SetMonData(&mon, MON_DATA_POKERUS, &pokerus);
    for (u32 stat = 0; stat < NUM_STATS; stat++)
        SetMonData(&mon, MON_DATA_HP_EV + stat, &ev);
    if (enabled)
        FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    else
        FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    MonGainEVs(&mon, SPECIES_CATERPIE);
    if (enabled)
    {
        for (u32 stat = 0; stat < NUM_STATS; stat++)
            EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_EV + stat), ev);
    }
    else
    {
        EXPECT_GT(GetMonData(&mon, MON_DATA_HP_EV), ev);
        if (item == ITEM_POWER_BRACER)
            EXPECT_GT(GetMonData(&mon, MON_DATA_ATK_EV), ev);
    }
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
}

TEST("No EV gain run rule survives saving and loading")
{
    bool32 enabled = FALSE;
    struct Pokemon mon;

    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }

    RunSetup_Begin();
    RunSetup_SetNoEVGain(enabled);
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    if (enabled)
        FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    else
        FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_NO_EV_GAIN), enabled);
    CreateRandomMonWithIVs(&mon, SPECIES_WOBBUFFET, 50, 0);
    MonGainEVs(&mon, SPECIES_CATERPIE);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_EV), enabled ? 0 : gSpeciesInfo[SPECIES_CATERPIE].evYield_HP);
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
}

TEST("No EV gain rejects all EV-raising items without changing EVs or friendship")
{
    struct Pokemon mon;
    bool32 enabled = FALSE;
    enum Item item = ITEM_NONE;
    u8 ev = 20;
    u8 friendship = 50;

    for (u32 rule = FALSE; rule <= TRUE; rule++)
    {
        for (enum Item vitamin = ITEM_HP_UP; vitamin <= ITEM_CARBOS; vitamin++)
            PARAMETRIZE { enabled = rule; item = vitamin; }
        for (enum Item feather = ITEM_HEALTH_FEATHER; feather <= ITEM_SWIFT_FEATHER; feather++)
            PARAMETRIZE { enabled = rule; item = feather; }
        for (enum Item mochi = ITEM_HEALTH_MOCHI; mochi <= ITEM_SWIFT_MOCHI; mochi++)
            PARAMETRIZE { enabled = rule; item = mochi; }
    }

    CreateRandomMonWithIVs(&mon, SPECIES_WOBBUFFET, 50, 0);
    SetMonData(&mon, MON_DATA_FRIENDSHIP, &friendship);
    for (u32 stat = 0; stat < NUM_STATS; stat++)
        SetMonData(&mon, MON_DATA_HP_EV + stat, &ev);
    if (enabled)
        FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    else
        FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    EXPECT_EQ(ExecuteTableBasedItemEffect(&mon, item, 0, 0), enabled);
    if (enabled)
    {
        for (u32 stat = 0; stat < NUM_STATS; stat++)
            EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_EV + stat), ev);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_FRIENDSHIP), friendship);
    }
    else
    {
        EXPECT_GT(GetMonEVCount(&mon), ev * NUM_STATS);
    }
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
}

TEST("No EV gain permits EV-reducing berries and EV resets")
{
    struct Pokemon mon;
    enum Item item = ITEM_NONE;
    u8 ev = 20;
    u8 friendship = 50;

    for (enum Item berry = ITEM_POMEG_BERRY; berry <= ITEM_TAMATO_BERRY; berry++)
        PARAMETRIZE { item = berry; }
    PARAMETRIZE { item = ITEM_FRESH_START_MOCHI; }

    CreateRandomMonWithIVs(&mon, SPECIES_WOBBUFFET, 50, 0);
    SetMonData(&mon, MON_DATA_FRIENDSHIP, &friendship);
    for (u32 stat = 0; stat < NUM_STATS; stat++)
        SetMonData(&mon, MON_DATA_HP_EV + stat, &ev);
    FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    EXPECT(!ExecuteTableBasedItemEffect(&mon, item, 0, 0));
    if (item == ITEM_FRESH_START_MOCHI)
        EXPECT_EQ(GetMonEVCount(&mon), 0);
    else
    {
        EXPECT_LT(GetMonEVCount(&mon), ev * NUM_STATS);
        EXPECT_GT(GetMonData(&mon, MON_DATA_FRIENDSHIP), friendship);
    }
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
}
