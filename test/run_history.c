#include "global.h"
#include "battle.h"
#include "main.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "run_history.h"
#include "save.h"
#include "test/test.h"

TEST("Run history preserves acquisitions without enforcing encounter rules")
{
    struct Pokemon mon;
    gMain.inBattle = FALSE;
    RunHistory_Init();
    CreateMonWithIVsPersonality(&mon, SPECIES_PIKACHU, 5, 10, 12345);
    RunHistory_RecordAcquisition(&mon);
    RunHistory_RecordAcquisition(&mon);
    EXPECT_EQ(gPokemonStoragePtr->runHistory.count, 2);
    EXPECT_EQ(gPokemonStoragePtr->runHistory.records[0].species, SPECIES_PIKACHU);
    EXPECT_EQ(gPokemonStoragePtr->runHistory.records[1].outcome, RUN_HISTORY_GIFT);
}

TEST("Run history preserves earliest records and reports overflow")
{
    struct Pokemon mon;
    gMain.inBattle = FALSE;
    RunHistory_Init();
    CreateMonWithIVsPersonality(&mon, SPECIES_RALTS, 5, 10, 12345);
    for (u32 i = 0; i < RUN_HISTORY_CAPACITY + 3; i++)
        RunHistory_RecordAcquisition(&mon);
    EXPECT_EQ(gPokemonStoragePtr->runHistory.count, RUN_HISTORY_CAPACITY);
    EXPECT_EQ(gPokemonStoragePtr->runHistory.dropped, 3);
    EXPECT_EQ(gPokemonStoragePtr->runHistory.records[0].personality, 12345);
    RunHistory_Init();
    EXPECT_EQ(gPokemonStoragePtr->runHistory.count, 0);
    EXPECT_EQ(gPokemonStoragePtr->runHistory.dropped, 0);
}

TEST("Run history distinguishes enemy knockouts from surviving enemies after a loss")
{
    RunHistory_Init();
    ZeroEnemyPartyMons();
    gBattleTypeFlags = BATTLE_TYPE_DOUBLE;
    gBattleOutcome = B_OUTCOME_LOST;
    CreateMonWithIVsPersonality(&gEnemyParty[0], SPECIES_RALTS, 5, 10, 1);
    CreateMonWithIVsPersonality(&gEnemyParty[1], SPECIES_PIKACHU, 5, 10, 2);
    u32 hp = 0;
    SetMonData(&gEnemyParty[0], MON_DATA_HP, &hp);
    RunHistory_RecordWildBattle();
    EXPECT_EQ(gPokemonStoragePtr->runHistory.count, 2);
    EXPECT_EQ(gPokemonStoragePtr->runHistory.records[0].outcome, RUN_HISTORY_KNOCKED_OUT);
    EXPECT_EQ(gPokemonStoragePtr->runHistory.records[1].outcome, RUN_HISTORY_LOST);
}

TEST("Run history records wild captures once and ignores trainer and tutorial battles")
{
    RunHistory_Init();
    ZeroEnemyPartyMons();
    gBattleTypeFlags = 0;
    gBattleOutcome = B_OUTCOME_CAUGHT;
    CreateMonWithIVsPersonality(&gEnemyParty[0], SPECIES_PIKACHU, 5, 10, 1);
    gMain.inBattle = TRUE;
    RunHistory_RecordAcquisition(&gEnemyParty[0]);
    EXPECT_EQ(gPokemonStoragePtr->runHistory.count, 0);
    RunHistory_RecordWildBattle();
    EXPECT_EQ(gPokemonStoragePtr->runHistory.count, 1);
    EXPECT_EQ(gPokemonStoragePtr->runHistory.records[0].outcome, RUN_HISTORY_CAUGHT);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    RunHistory_RecordWildBattle();
    gBattleTypeFlags = BATTLE_TYPE_CATCH_TUTORIAL;
    RunHistory_RecordWildBattle();
    EXPECT_EQ(gPokemonStoragePtr->runHistory.count, 1);
    gMain.inBattle = FALSE;
}

TEST("Run history records split evolution acquisitions separately")
{
    struct Pokemon mon;
    RunHistory_Init();
    CreateMonWithIVsPersonality(&mon, SPECIES_SHEDINJA, 20, 10, 12345);
    RunHistory_RecordSplitEvolution(&mon);
    EXPECT_EQ(gPokemonStoragePtr->runHistory.count, 1);
    EXPECT_EQ(gPokemonStoragePtr->runHistory.records[0].species, SPECIES_SHEDINJA);
    EXPECT_EQ(gPokemonStoragePtr->runHistory.records[0].outcome, RUN_HISTORY_SPLIT_EVOLUTION);
}

TEST("Run history and discovery survive a full save and load at capacity")
{
    struct Pokemon mon;
    struct RunHistory expected;
    struct RunDiscovery discovery;
    gMain.inBattle = FALSE;
    RunHistory_Init();
    CreateMonWithIVsPersonality(&mon, SPECIES_PIKACHU, 20, 10, 12345);
    for (u32 i = 0; i < RUN_HISTORY_CAPACITY + 1; i++)
        RunHistory_RecordAcquisition(&mon);
    RunHistory_RecordTutor(MOVE_DOUBLE_EDGE);
    expected = gPokemonStoragePtr->runHistory;
    discovery = gSaveBlock1Ptr->runDiscovery;
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    memset(&gPokemonStoragePtr->runHistory, 0, sizeof(expected));
    memset(&gSaveBlock1Ptr->runDiscovery, 0, sizeof(discovery));
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(memcmp(&gPokemonStoragePtr->runHistory, &expected, sizeof(expected)), 0);
    EXPECT_EQ(memcmp(&gSaveBlock1Ptr->runDiscovery, &discovery, sizeof(discovery)), 0);
}
