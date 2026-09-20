#include "global.h"
#include "battle.h"
#include "battle_transition.h"
#include "data.h"
#include "event_data.h"
#include "pokemon.h"
#include "save.h"
#include "species_randomizer.h"
#include "trainer_randomizer.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/battle_ai.h"
#include "constants/pokeball.h"

const struct Trainer gRunTrainerTestData[DIFFICULTY_COUNT][TRAINERS_COUNT] =
{
#include "../src/data/trainers.h"
};

TEST("Trainer randomizer keeps authored levels IVs items and effective default moves")
{
    const u16 trainers[] = {TRAINER_CALVIN_1, TRAINER_ROXANNE_1, TRAINER_TATE_AND_LIZA_1, TRAINER_WALLY_VR_1, TRAINER_WALLACE};
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    FlagSet(FLAG_RUN_RULE_LEARNSETS);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    for (u32 t = 0; t < ARRAY_COUNT(trainers); t++)
    {
        const struct Trainer *original = GetRunTrainer(trainers[t]);
        EXPECT(CreateRunTrainerParty(gParties[B_TRAINER_OPPONENT_A], trainers[t]));
        for (u32 i = 0; i < original->partySize; i++)
        {
            struct Pokemon expected = gParties[B_TRAINER_OPPONENT_A][i];
            u32 species = GetMonData(&expected, MON_DATA_SPECIES);
            EXPECT(IsRandomizerSpeciesEligible(species));
            EXPECT(!IsRandomizerSpeciesLegendary(species));
            EXPECT_EQ(GetMonData(&expected, MON_DATA_LEVEL), original->party[i].lvl);
            EXPECT_EQ(GetMonData(&expected, MON_DATA_IVS), original->party[i].iv);
            EXPECT_EQ(GetMonData(&expected, MON_DATA_HELD_ITEM), original->party[i].heldItem);
            GiveMonInitialMoveset(&expected);
            for (u32 j = 0; j < 4; j++)
                EXPECT_EQ(GetMonData(&expected, MON_DATA_MOVE1 + j), GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_MOVE1 + j));
        }
        if (original->partySize < PARTY_SIZE)
            EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][original->partySize], MON_DATA_SPECIES), SPECIES_NONE);
    }
}

TEST("Trainer randomizer follows all rival starter branches and keeps retries stable")
{
    const u16 trainers[] = {TRAINER_BRENDAN_ROUTE_103_TREECKO, TRAINER_BRENDAN_ROUTE_103_TORCHIC, TRAINER_BRENDAN_ROUTE_103_MUDKIP,
        TRAINER_MAY_ROUTE_103_TREECKO, TRAINER_MAY_ROUTE_103_TORCHIC, TRAINER_MAY_ROUTE_103_MUDKIP};
    const u16 starters[] = {SPECIES_TORCHIC, SPECIES_MUDKIP, SPECIES_TREECKO};
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    FlagSet(FLAG_RUN_RULE_ENCOUNTERS);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    for (u32 i = 0; i < ARRAY_COUNT(trainers); i++)
    {
        EXPECT(CreateRunTrainerParty(gParties[B_TRAINER_OPPONENT_A], trainers[i]));
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES), RandomizeStarterSpecies(starters[i % 3], (i + 1) % 3));
    }
    for (u32 id = 1; id < TRAINERS_COUNT; id++)
    {
        const struct Trainer *trainer = GetRunTrainer(id);
        if (trainer->trainerClass != TRAINER_CLASS_RIVAL || (trainer->trainerPic != TRAINER_PIC_BRENDAN && trainer->trainerPic != TRAINER_PIC_MAY))
            continue;
        for (u32 slot = 0; slot < trainer->partySize; slot++)
        {
            u32 original = trainer->party[slot].species;
            u32 starter = original == SPECIES_GROVYLE || original == SPECIES_TREECKO ? 0
                : original == SPECIES_COMBUSKEN || original == SPECIES_TORCHIC ? 1
                : original == SPECIES_MARSHTOMP || original == SPECIES_MUDKIP ? 2 : 3;
            if (starter < 3)
            {
                const u16 bases[] = {SPECIES_TREECKO, SPECIES_TORCHIC, SPECIES_MUDKIP};
                u16 chosen = RandomizeStarterSpecies(bases[starter], starter);
                EXPECT_EQ(GetRandomizerSpeciesFamily(GetRunTrainerSpecies(id, slot, original)), GetRandomizerSpeciesFamily(chosen));
            }
        }
    }
    EXPECT(CreateRunTrainerParty(gParties[B_TRAINER_OPPONENT_A], TRAINER_ROXANNE_1));
    u16 species[PARTY_SIZE];
    for (u32 i = 0; i < PARTY_SIZE; i++)
        species[i] = GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_SPECIES);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    gSaveBlock2Ptr->playerTrainerId[0] ^= 1;
    EXPECT(CreateRunTrainerParty(gParties[B_TRAINER_OPPONENT_A], TRAINER_ROXANNE_1));
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT(CreateRunTrainerParty(gParties[B_TRAINER_OPPONENT_A], TRAINER_ROXANNE_1));
    for (u32 i = 0; i < PARTY_SIZE; i++)
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_SPECIES), species[i]);
}

TEST("Trainer randomizer excludes special battles and keeps two opponent parties separate")
{
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    const u32 excluded[] = {BATTLE_TYPE_LINK, BATTLE_TYPE_FRONTIER, BATTLE_TYPE_CATCH_TUTORIAL, BATTLE_TYPE_TRAINER_HILL, BATTLE_TYPE_SECRET_BASE};
    for (u32 i = 0; i < ARRAY_COUNT(excluded); i++)
    {
        gBattleTypeFlags = BATTLE_TYPE_TRAINER | excluded[i];
        EXPECT(!CreateRunTrainerParty(gParties[B_TRAINER_OPPONENT_A], TRAINER_ROXANNE_1));
    }
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_TWO_OPPONENTS | BATTLE_TYPE_DOUBLE;
    EXPECT(CreateRunTrainerParty(gParties[B_TRAINER_OPPONENT_A], TRAINER_ROXANNE_1));
    u16 first = GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES);
    EXPECT(CreateRunTrainerParty(gParties[B_TRAINER_OPPONENT_B], TRAINER_BRAWLY_1));
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES), first);
    EXPECT(!IsRunTrainerBattle(TRAINER_PARTNER(PARTNER_STEVEN)));
    FlagClear(FLAG_RUN_RULE_TRAINERS);
    EXPECT(!CreateRunTrainerParty(gParties[B_TRAINER_OPPONENT_A], TRAINER_ROXANNE_1));
}

TEST("Trainer randomizer disabled Tera overrides forced species types")
{
    struct Pokemon mon;
    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        if (!gSpeciesInfo[species].baseHP || !gSpeciesInfo[species].forceTeraType)
            continue;
        CreateMon(&mon, species, 50, 0, OTID_STRUCT_PLAYER_ID);
        u32 type = TYPE_MYSTERY;
        SetMonData(&mon, MON_DATA_TERA_TYPE, &type);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_TERA_TYPE), TYPE_MYSTERY);
        type = TYPE_NONE;
        SetMonData(&mon, MON_DATA_TERA_TYPE, &type);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_TERA_TYPE), gSpeciesInfo[species].forceTeraType);
    }
}
