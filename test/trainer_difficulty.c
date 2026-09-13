#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "pokemon.h"
#include "save.h"
#include "species_randomizer.h"
#include "trainer_difficulty.h"
#include "trainer_randomizer.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/form_change_types.h"
#include "constants/vars.h"

TEST("Trainer difficulty applies the level IV and party size matrix to campaign templates")
{
    u32 difficulty = 0;
    bool32 randomized = FALSE;
    for (u32 d = 0; d <= 2; d++)
        for (u32 r = 0; r < 2; r++)
            PARAMETRIZE { difficulty = d; randomized = r; }
    const u16 trainers[] = {TRAINER_CALVIN_1, TRAINER_ROXANNE_1, TRAINER_TATE_AND_LIZA_1,
        TRAINER_WALLY_MAUVILLE, TRAINER_WALLY_VR_1, TRAINER_BRENDAN_ROUTE_103_TREECKO};
    VarSet(VAR_RUN_RULE_DIFFICULTY, difficulty);
    if (randomized)
        FlagSet(FLAG_RUN_RULE_TRAINERS);
    else
        FlagClear(FLAG_RUN_RULE_TRAINERS);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    for (u32 t = 0; t < ARRAY_COUNT(trainers); t++)
    {
        u32 id = trainers[t];
        const struct Trainer *trainer = GetRunTrainer(id);
        bool32 boss = t == 1 || t == 2 || t == 4;
        EXPECT_EQ(IsRunTrainerBoss(id), boss);
        u32 count = randomized && difficulty ? (boss ? 6 : min(6, trainer->partySize + difficulty)) : trainer->partySize;
        EXPECT_EQ(GetRunTrainerPartySize(id, trainer->partySize), count);
        EXPECT_EQ(CreateRunTrainerParty(gParties[B_TRAINER_OPPONENT_A], id), randomized || difficulty);
        if (!randomized && !difficulty)
            continue;
        u32 lowest = 0;
        for (u32 i = 1; i < trainer->partySize; i++)
            if (trainer->party[i].lvl < trainer->party[lowest].lvl)
                lowest = i;
        for (u32 i = 0; i < count; i++)
        {
            struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][i];
            const struct TrainerMon *entry = &trainer->party[i < trainer->partySize ? i : lowest];
            EXPECT_EQ(GetMonData(mon, MON_DATA_LEVEL), entry->lvl + (difficulty == 2 ? 2 : difficulty == 1 && !boss));
            EXPECT_EQ(GetMonData(mon, MON_DATA_IVS), difficulty ? TRAINER_PARTY_IVS(31, 31, 31, 31, 31, 31) : entry->iv);
            if (!randomized)
                EXPECT_EQ(GetMonData(mon, MON_DATA_SPECIES), entry->species);
        }
        if (count < PARTY_SIZE)
            EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][count], MON_DATA_SPECIES), SPECIES_NONE);
    }
    EXPECT_EQ(GetRunTrainerLevel(TRAINER_CALVIN_1, 100), 100);
}

TEST("Trainer difficulty special rolls are independent, capped and protect the rival starter")
{
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    const u16 trainers[] = {TRAINER_CALVIN_1, TRAINER_ROXANNE_1};
    for (u32 d = 0; d < 3; d++)
    {
        VarSet(VAR_RUN_RULE_DIFFICULTY, d);
        for (u32 t = 0; t < ARRAY_COUNT(trainers); t++)
        {
            u32 legends = 0, megas = 0, both = 0;
            for (u32 seed = 0; seed < 256; seed++)
            {
                gSaveBlock2Ptr->playerTrainerId[0] = seed;
                u32 legendary, mega;
                GetRunTrainerSpecialSlots(trainers[t], 6, 0, &legendary, &mega);
                EXPECT_NE(legendary, 0);
                EXPECT_NE(mega, 0);
                if (legendary < PARTY_SIZE && mega < PARTY_SIZE)
                {
                    EXPECT_NE(legendary, mega);
                    both++;
                }
                legends += legendary < PARTY_SIZE;
                megas += mega < PARTY_SIZE;
            }
            if (d == 0 || (d == 1 && t == 0))
            {
                EXPECT_EQ(legends, 0);
                EXPECT_EQ(megas, 0);
            }
            else
            {
                EXPECT_GT(legends, 0);
                EXPECT_GT(megas, 0);
                EXPECT_GT(both, 0);
            }
        }
    }
}

TEST("Trainer difficulty generates compatible Mega equipment despite item bans and zeros EVs")
{
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    FlagSet(FLAG_RUN_RULE_ITEMS);
    FlagSet(FLAG_RUN_RULE_BAN_MEGA_STONES);
    FlagSet(FLAG_RUN_RULE_BAN_Z_CRYSTALS);
    FlagSet(FLAG_RUN_RULE_BAN_TERA_SHARDS);
    FlagSet(FLAG_RUN_RULE_BAN_TYPE_GEMS);
    FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    VarSet(VAR_RUN_RULE_DIFFICULTY, RUN_TRAINER_UNFAIR);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    u32 legendary, mega;
    for (u32 seed = 0; seed < 256; seed++)
    {
        gSaveBlock2Ptr->playerTrainerId[0] = seed;
        GetRunTrainerSpecialSlots(TRAINER_ROXANNE_1, 6, PARTY_SIZE, &legendary, &mega);
        if (mega < PARTY_SIZE)
            break;
    }
    EXPECT_LT(mega, PARTY_SIZE);
    TRAINER_BATTLE_PARAM.opponentA = TRAINER_ROXANNE_1;
    EXPECT(CreateRunTrainerParty(gParties[B_TRAINER_OPPONENT_A], TRAINER_ROXANNE_1));
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][mega];
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    u16 item = GetMonData(mon, MON_DATA_HELD_ITEM);
    EXPECT(!IsRandomizerSpeciesLegendary(species));
    bool32 compatible = FALSE;
    const struct FormChange *changes = GetSpeciesFormChanges(species);
    for (u32 i = 0; changes != NULL && changes[i].method != FORM_CHANGE_TERMINATOR; i++)
        compatible |= changes[i].method == FORM_CHANGE_BATTLE_MEGA_EVOLUTION_ITEM && changes[i].param1 == item;
    EXPECT(compatible);
    for (u32 slot = 0; slot < PARTY_SIZE; slot++)
        EXPECT_EQ(CanRunTrainerSlotMegaEvolve(B_TRAINER_OPPONENT_A, slot), slot == mega);
    for (u32 i = 0; i < PARTY_SIZE; i++)
        for (u32 stat = 0; stat < NUM_STATS; stat++)
            EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_HP_EV + stat), 0);
}

TEST("Trainer difficulty reproduces expanded teams and equipment across save load")
{
    u16 values[PARTY_SIZE][7];
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    FlagSet(FLAG_RUN_RULE_ITEMS);
    VarSet(VAR_RUN_RULE_DIFFICULTY, RUN_TRAINER_UNFAIR);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TWO_OPPONENTS;
    EXPECT(CreateRunTrainerParty(gParties[B_TRAINER_OPPONENT_A], TRAINER_TATE_AND_LIZA_1));
    for (u32 i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][i];
        values[i][0] = GetMonData(mon, MON_DATA_SPECIES);
        values[i][1] = GetMonData(mon, MON_DATA_HELD_ITEM);
        values[i][2] = GetMonData(mon, MON_DATA_LEVEL);
        for (u32 j = 0; j < 4; j++)
            values[i][3 + j] = GetMonData(mon, MON_DATA_MOVE1 + j);
    }
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    gSaveBlock2Ptr->playerTrainerId[0] ^= 1;
    VarSet(VAR_RUN_RULE_DIFFICULTY, RUN_TRAINER_NORMAL);
    EXPECT(CreateRunTrainerParty(gParties[B_TRAINER_OPPONENT_A], TRAINER_TATE_AND_LIZA_1));
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT(CreateRunTrainerParty(gParties[B_TRAINER_OPPONENT_A], TRAINER_TATE_AND_LIZA_1));
    for (u32 i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][i];
        EXPECT_EQ(GetMonData(mon, MON_DATA_SPECIES), values[i][0]);
        EXPECT_EQ(GetMonData(mon, MON_DATA_HELD_ITEM), values[i][1]);
        EXPECT_EQ(GetMonData(mon, MON_DATA_LEVEL), values[i][2]);
        for (u32 j = 0; j < 4; j++)
            EXPECT_EQ(GetMonData(mon, MON_DATA_MOVE1 + j), values[i][3 + j]);
    }
}

TEST("Trainer difficulty blocks incidental item and move Mega evolution outside its special slot")
{
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    TRAINER_BATTLE_PARAM.opponentA = TRAINER_ROXANNE_1;
    EXPECT(CreateRunTrainerParty(gParties[B_TRAINER_OPPONENT_A], TRAINER_ROXANNE_1));
    for (u32 i = 0; i < PARTY_SIZE; i++)
        EXPECT(!CanRunTrainerSlotMegaEvolve(B_TRAINER_OPPONENT_A, i));
    EXPECT(CanRunTrainerSlotMegaEvolve(B_TRAINER_PLAYER, 0));
    EXPECT(CanRunTrainerSlotMegaEvolve(B_TRAINER_PARTNER, 0));
    FlagClear(FLAG_RUN_RULE_TRAINERS);
    EXPECT(CanRunTrainerSlotMegaEvolve(B_TRAINER_OPPONENT_A, 0));
}
