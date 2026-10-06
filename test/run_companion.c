#include "global.h"
#include "event_data.h"
#include "item.h"
#include "load_save.h"
#include "string_util.h"
#include "gba/flash_internal.h"
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
    EXPECT_EQ(gPokemonStoragePtr->companion.magic, COMPANION_SAVE_MAGIC);
    EXPECT_EQ(gPokemonStoragePtr->companion.layoutHash, COMPANION_SAVE_LAYOUT_HASH);
    RunCompanion_RecordTutor(MOVE_DOUBLE_EDGE);
    EXPECT(gSaveBlock1Ptr->runDiscovery.tutorsSeen != 0);
    expected = gPokemonStoragePtr->companion;
    discovery = gSaveBlock1Ptr->runDiscovery;
    ClearSaveData();
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

// The companion's ROM save vector. Values here are mirrored in the companion's test/save.test.ts.
// `make check TESTS="Companion save vector"` also dumps both save slots for the companion's scripts/dump_rom_save.py.
TEST("Companion save vector")
{
    struct Pokemon mon;
    u32 trainerId = 0x2468ACE1, value;
    memcpy(gSaveBlock2Ptr->playerTrainerId, &trainerId, sizeof(trainerId));
    StringCopy(gSaveBlock2Ptr->playerName, COMPOUND_STRING("TESTER"));
    gSaveBlock2Ptr->playTimeHours = 12;
    gSaveBlock2Ptr->playTimeMinutes = 34;
    gSaveBlock2Ptr->encryptionKey = 0x13572468;
    RunCompanion_Init();
    gPokemonStoragePtr->companion.runId = 0xC0FFEE;
    FlagSet(FLAG_RUN_RULE_ABILITIES);
    ClearBag();
    AddBagItem(ITEM_POKE_BALL, 5);
    AddBagItem(ITEM_POTION, 3);
    AddBagItem(ITEM_RARE_CANDY, 7);

    ZeroPlayerPartyMons();
    CreateMon(&mon, SPECIES_PIKACHU, 25, 0x00001234, OTID_STRUCT_PLAYER_ID);
    SetMonData(&mon, MON_DATA_NICKNAME, COMPOUND_STRING("SPARKY"));
    value = NATURE_ADAMANT;
    SetMonData(&mon, MON_DATA_HIDDEN_NATURE, &value);
    value = TRUE;
    SetMonData(&mon, MON_DATA_IS_SHINY, &value);
    SetMonData(&mon, MON_DATA_HYPER_TRAINED_HP, &value);
    value = 252;
    SetMonData(&mon, MON_DATA_ATK_EV, &value);
    value = ITEM_LEFTOVERS;
    SetMonData(&mon, MON_DATA_HELD_ITEM, &value);
    CalculateMonStats(&mon);
    value = GetMonData(&mon, MON_DATA_MAX_HP) - 10;
    SetMonData(&mon, MON_DATA_HP, &value);
    gPlayerParty[0] = mon;
    SetBoxMonAt(0, 0, &mon.box);
    CreateMon(&gPlayerParty[1], SPECIES_RALTS, 10, 0x00005678, OTID_STRUCT_PLAYER_ID);
    SetBoxMonAt(TOTAL_BOXES_COUNT - 1, IN_BOX_COUNT - 1, &gPlayerParty[1].box);
    gPlayerPartyCount = 2;
    SavePlayerParty();

    ClearSaveData();
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    if (strcmp(gTestRunnerArgv, "Companion save vector") == 0)
    {
        static const char digits[] = "0123456789abcdef";
        u8 bytes[64];
        char hex[sizeof(bytes) * 2 + 1];
        for (u32 sector = 0; sector < NUM_SECTORS_PER_SLOT * 2; sector++)
        {
            for (u32 offset = 0; offset < SECTOR_SIZE; offset += sizeof(bytes))
            {
                ReadFlash(sector, offset, bytes, sizeof(bytes));
                for (u32 i = 0; i < sizeof(bytes); i++)
                {
                    hex[i * 2] = digits[bytes[i] >> 4];
                    hex[i * 2 + 1] = digits[bytes[i] & 15];
                }
                hex[sizeof(hex) - 1] = '\0';
                Test_MgbaPrintf("save %d %d %s", sector, offset, hex);
            }
        }
    }
}
