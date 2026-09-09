#include "global.h"
#include "caps.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "storage_level_cap.h"
#include "test/test.h"
#include "constants/flags.h"

extern void Test_CreateShedinja(enum Species before, enum Species after, struct Pokemon *mon);

static void SetUpStorageCap(u32 partySize)
{
    StorageLevelCap_Finish();
    FlagSet(FLAG_RUN_RULE_LEVEL_CAPS);
    for (u32 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
        FlagClear(flag);
    FlagSet(FLAG_BADGE01_GET);
    FlagSet(FLAG_BADGE02_GET);
    FlagClear(FLAG_IS_CHAMPION);
    memset(gPokemonStoragePtr->boxes, 0, sizeof(gPokemonStoragePtr->boxes));
    for (u32 i = 0; i < PARTY_SIZE; i++)
    {
        ZeroMonData(&gParties[B_TRAINER_PLAYER][i]);
        if (i < partySize)
            CreateRandomMon(&gParties[B_TRAINER_PLAYER][i], SPECIES_WOBBUFFET, 10);
    }
    CalculatePlayerPartyCount();
    ClearBag();
}

TEST("PC level to cap restores a full or partial party and writes back to the original box slot")
{
    u32 partySize = 0;
    struct Pokemon original[PARTY_SIZE], mon;
    PARAMETRIZE { partySize = 2; }
    PARAMETRIZE { partySize = PARTY_SIZE; }
    SetUpStorageCap(partySize);
    memcpy(original, gParties[B_TRAINER_PLAYER], sizeof(original));
    CreateRandomMon(&mon, SPECIES_TREECKO, 5);
    SetBoxMonAt(3, 17, &mon.box);
    EXPECT(StorageLevelCap_Begin(TRUE, 3, 17));
    EXPECT(!StorageLevelCap_Begin(TRUE, 3, 17));
    struct Pokemon *working = &gParties[B_TRAINER_PLAYER][StorageLevelCap_GetPartySlot()];
    EXPECT(RaiseMonToLevelCap(working));
    EXPECT_EQ(GetMonData(working, MON_DATA_LEVEL), 24);
    EXPECT_EQ(GetBoxMonDataAt(3, 17, MON_DATA_EXP), GetMonData(&mon, MON_DATA_EXP));
    u32 intermediate = 8;
    SetMonData(working, MON_DATA_LEVEL, &intermediate);
    StorageLevelCap_Finish();
    EXPECT_EQ(memcmp(original, gParties[B_TRAINER_PLAYER], sizeof(original)), 0);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], partySize);
    EXPECT_EQ(GetBoxMonDataAt(3, 17, MON_DATA_EXP), gExperienceTables[gSpeciesInfo[SPECIES_TREECKO].growthRate][24]);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    ZeroBoxMonAt(3, 17);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(GetBoxMonDataAt(3, 17, MON_DATA_EXP), gExperienceTables[gSpeciesInfo[SPECIES_TREECKO].growthRate][24]);
    FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
}

TEST("PC level to cap uses real party members for boxed evolution conditions")
{
    struct Pokemon mon;
    bool32 canStop;
    SetUpStorageCap(PARTY_SIZE);
    CreateRandomMon(&gParties[B_TRAINER_PLAYER][5], SPECIES_REMORAID, 10);
    CreateRandomMon(&mon, SPECIES_MANTYKE, 10);
    SetBoxMonAt(0, 0, &mon.box);
    EXPECT(StorageLevelCap_Begin(TRUE, 0, 0));
    struct Pokemon *working = &gParties[B_TRAINER_PLAYER][StorageLevelCap_GetPartySlot()];
    EXPECT_EQ(GetEvolutionTargetSpecies(working, EVO_MODE_NORMAL, ITEM_NONE, NULL, &canStop, CHECK_EVO), SPECIES_MANTINE);
    StorageLevelCap_Finish();
    CreateRandomMon(&gParties[B_TRAINER_PLAYER][5], SPECIES_POOCHYENA, 10);
    CreateRandomMon(&mon, SPECIES_PANCHAM, 40);
    SetBoxMonAt(0, 0, &mon.box);
    EXPECT(StorageLevelCap_Begin(TRUE, 0, 0));
    working = &gParties[B_TRAINER_PLAYER][StorageLevelCap_GetPartySlot()];
    EXPECT_EQ(GetEvolutionTargetSpecies(working, EVO_MODE_NORMAL, ITEM_NONE, NULL, &canStop, CHECK_EVO), SPECIES_PANGORO);
    StorageLevelCap_Finish();
    FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
}

TEST("PC level to cap sends boxed Shedinja to the first free PC slot without changing the party")
{
    bool32 space = FALSE, ball = FALSE;
    u32 partySize = 0;
    for (u32 size = PARTY_SIZE - 1; size <= PARTY_SIZE; size++)
        for (u32 free = FALSE; free <= TRUE; free++)
            for (u32 hasBall = FALSE; hasBall <= TRUE; hasBall++)
                PARAMETRIZE { space = free; ball = hasBall; partySize = size; }
    struct Pokemon original[PARTY_SIZE], mon;
    SetUpStorageCap(partySize);
    memcpy(original, gParties[B_TRAINER_PLAYER], sizeof(original));
    CreateRandomMon(&mon, SPECIES_WOBBUFFET, 10);
    for (u32 box = 0; box < TOTAL_BOXES_COUNT; box++)
        for (u32 slot = 0; slot < IN_BOX_COUNT; slot++)
            SetBoxMonAt(box, slot, &mon.box);
    if (space)
        ZeroBoxMonAt(0, 7);
    CreateRandomMon(&mon, SPECIES_NINCADA, 20);
    SetBoxMonAt(3, 17, &mon.box);
    if (ball)
        AddBagItem(ITEM_POKE_BALL, 1);
    EXPECT(StorageLevelCap_Begin(TRUE, 3, 17));
    struct Pokemon *working = &gParties[B_TRAINER_PLAYER][StorageLevelCap_GetPartySlot()];
    u32 species = SPECIES_NINJASK;
    SetMonData(working, MON_DATA_SPECIES, &species);
    Test_CreateShedinja(SPECIES_NINCADA, SPECIES_NINJASK, working);
    StorageLevelCap_Finish();
    EXPECT_EQ(GetBoxMonDataAt(3, 17, MON_DATA_SPECIES), SPECIES_NINJASK);
    EXPECT_EQ(GetBoxMonDataAt(0, 7, MON_DATA_SPECIES), space ? (ball ? SPECIES_SHEDINJA : SPECIES_NONE) : SPECIES_WOBBUFFET);
    EXPECT_EQ(CheckBagHasItem(ITEM_POKE_BALL, 1), ball && !space);
    EXPECT_EQ(memcmp(original, gParties[B_TRAINER_PLAYER], sizeof(original)), 0);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], partySize);
    FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
}

TEST("PC level to cap keeps normal party Shedinja creation rules")
{
    u32 size = 0;
    PARAMETRIZE { size = PARTY_SIZE - 1; }
    PARAMETRIZE { size = PARTY_SIZE; }
    SetUpStorageCap(size);
    CreateRandomMon(&gParties[B_TRAINER_PLAYER][0], SPECIES_NINJASK, 20);
    AddBagItem(ITEM_POKE_BALL, 1);
    EXPECT(StorageLevelCap_Begin(FALSE, 0, 0));
    Test_CreateShedinja(SPECIES_NINCADA, SPECIES_NINJASK, &gParties[B_TRAINER_PLAYER][0]);
    StorageLevelCap_Finish();
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][5], MON_DATA_SPECIES), size == 5 ? SPECIES_SHEDINJA : SPECIES_WOBBUFFET);
    EXPECT_EQ(GetBoxMonDataAt(0, 0, MON_DATA_SPECIES), SPECIES_NONE);
    EXPECT_EQ(CheckBagHasItem(ITEM_POKE_BALL, 1), size == PARTY_SIZE);
    FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
}

TEST("PC level to cap rejects Eggs, invalid slots and disabled rules")
{
    struct Pokemon mon;
    SetUpStorageCap(1);
    EXPECT(!StorageLevelCap_Begin(TRUE, TOTAL_BOXES_COUNT, 0));
    EXPECT(!StorageLevelCap_Begin(TRUE, 0, IN_BOX_COUNT));
    EXPECT(!StorageLevelCap_Begin(TRUE, 0, 0));
    EXPECT(!StorageLevelCap_Begin(FALSE, 0, PARTY_SIZE));
    CreateRandomMon(&mon, SPECIES_TREECKO, 5);
    u32 egg = TRUE;
    SetMonData(&mon, MON_DATA_IS_EGG, &egg);
    SetBoxMonAt(0, 0, &mon.box);
    EXPECT(!StorageLevelCap_Begin(TRUE, 0, 0));
    FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
    EXPECT(!StorageLevelCap_Begin(FALSE, 0, 0));
}

TEST("PC level to cap appears in both PC views without dropping Cancel and excludes Eggs and carried Pokemon")
{
    bool32 inParty = FALSE, carrying = FALSE, egg = FALSE, enabled = FALSE;
    for (u32 party = FALSE; party <= TRUE; party++)
        for (u32 held = FALSE; held <= TRUE; held++)
            for (u32 isEgg = FALSE; isEgg <= TRUE; isEgg++)
                for (u32 rule = FALSE; rule <= TRUE; rule++)
                    PARAMETRIZE { inParty = party; carrying = held; egg = isEgg; enabled = rule; }
    SetUpStorageCap(1);
    SetBoxMonAt(StorageGetCurrentBox(), 0, &gParties[B_TRAINER_PLAYER][0].box);
    if (!enabled)
        FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
    EXPECT_EQ(Test_StorageLevelCapMenu(inParty, carrying, egg), enabled && !carrying && !egg ? 3 : 2);
    FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
}

TEST("PC level to cap leaves capped and overleveled boxed Pokemon unchanged")
{
    struct Pokemon mon;
    u32 level = 0;
    PARAMETRIZE { level = 24; }
    PARAMETRIZE { level = 50; }
    SetUpStorageCap(PARTY_SIZE);
    CreateRandomMon(&mon, SPECIES_TREECKO, level);
    SetBoxMonAt(0, 0, &mon.box);
    EXPECT(StorageLevelCap_Begin(TRUE, 0, 0));
    EXPECT(!RaiseMonToLevelCap(&gParties[B_TRAINER_PLAYER][StorageLevelCap_GetPartySlot()]));
    StorageLevelCap_Finish();
    EXPECT_EQ(memcmp(GetBoxedMonPtr(0, 0), &mon.box, sizeof(mon.box)), 0);
    FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
}

TEST("PC level to cap cannot grant a boxed Pokemon the active follower's walking progress")
{
    struct Pokemon mon;
    bool32 canStop;
    SetUpStorageCap(0);
    CreateRandomMon(&mon, SPECIES_PAWMO, 30);
    SetBoxMonAt(0, 0, &mon.box);
    gFollowerSteps = 1000;
    EXPECT(StorageLevelCap_Begin(TRUE, 0, 0));
    EXPECT_EQ(GetEvolutionTargetSpecies(&gParties[B_TRAINER_PLAYER][0], EVO_MODE_NORMAL, ITEM_NONE, NULL, &canStop, CHECK_EVO), SPECIES_NONE);
    StorageLevelCap_Finish();
    gFollowerSteps = 0;
    FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
}

TEST("PC level to cap screen teardown stops the outgoing frame before sprite callbacks")
{
    EXPECT(Test_StorageFrameStopsAfterTeardown());
}
