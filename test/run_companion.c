#include "global.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "run_companion.h"
#include "save.h"
#include "test/test.h"

TEST("Companion metadata and tutor discovery survive a full save and load")
{
    struct CompanionSaveMetadata expected;
    struct RunDiscovery discovery;
    RunCompanion_Init();
    EXPECT_EQ(sizeof(struct CompanionSaveMetadata), 12);
    EXPECT_EQ(sizeof(struct RunDiscovery), 36);
    EXPECT_EQ(gPokemonStoragePtr->companion.magic, COMPANION_SAVE_MAGIC);
    EXPECT_EQ(gPokemonStoragePtr->companion.version, COMPANION_SAVE_VERSION);
    RunCompanion_RecordTutor(MOVE_DOUBLE_EDGE);
    EXPECT(gSaveBlock1Ptr->runDiscovery.tutorsSeen != 0);
    expected = gPokemonStoragePtr->companion;
    discovery = gSaveBlock1Ptr->runDiscovery;
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    memset(&gPokemonStoragePtr->companion, 0, sizeof(expected));
    memset(&gSaveBlock1Ptr->runDiscovery, 0, sizeof(discovery));
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(memcmp(&gPokemonStoragePtr->companion, &expected, sizeof(expected)), 0);
    EXPECT_EQ(memcmp(&gSaveBlock1Ptr->runDiscovery, &discovery, sizeof(discovery)), 0);
    RunCompanion_Init();
    EXPECT_EQ(gSaveBlock1Ptr->runDiscovery.tutorsSeen, 0);
}
