#include "global.h"
#include "enemy_stab.h"
#include "event_data.h"
#include "move.h"
#include "move_randomizer.h"
#include "pokemon.h"
#include "run_randomizer.h"
#include "save.h"
#include "test/test.h"
#include "constants/flags.h"

TEST("Enemy STAB upgrades weak single type coverage with minimal replacements and stable PP")
{
    struct Pokemon mon;
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    FlagSet(FLAG_RUN_RULE_ENEMY_STAB);
    CreateMon(&mon, SPECIES_ARCANINE, 10, 0, OTID_STRUCT_PLAYER_ID);
    SetMonMoveSlot(&mon, MOVE_EMBER, 0);
    SetMonMoveSlot(&mon, MOVE_GROWL, 1);
    SetMonMoveSlot(&mon, MOVE_TACKLE, 2);
    SetMonMoveSlot(&mon, MOVE_SCRATCH, 3);
    EnsureEnemyStabMoves(&mon, TRAINER_ROXANNE_1, 0);
    u16 move = GetMonData(&mon, MON_DATA_MOVE1);
    EXPECT(IsEnemyStabMove(move, TYPE_FIRE));
    EXPECT(move != MOVE_EMBER);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE2), MOVE_GROWL);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE3), MOVE_TACKLE);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE4), MOVE_SCRATCH);
    EXPECT(GetRandomizerMovePower(move) >= 50);
    EXPECT_LE(GetRandomizerMovePower(move), 80);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_PP1), GetMovePP(move));
    EnsureEnemyStabMoves(&mon, TRAINER_ROXANNE_1, 0);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE1), move);
}

TEST("Enemy STAB guarantees natural coverage and leaves disabled rules unchanged")
{
    struct Pokemon mon;
    CreateMon(&mon, SPECIES_GENGAR, 50, 0, OTID_STRUCT_PLAYER_ID);
    for (u32 i = 0; i < 4; i++)
        SetMonMoveSlot(&mon, MOVE_GROWL, i);
    FlagSet(FLAG_RUN_RULE_ENEMY_STAB);
    EnsureEnemyStabMoves(&mon, TRAINER_ROXANNE_1, 0);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE1), MOVE_GROWL);
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    FlagClear(FLAG_RUN_RULE_ENEMY_STAB);
    EnsureEnemyStabMoves(&mon, TRAINER_ROXANNE_1, 0);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE1), MOVE_GROWL);
    FlagSet(FLAG_RUN_RULE_ENEMY_STAB);
    EnsureEnemyStabMoves(&mon, TRAINER_ROXANNE_1, 0);
    bool32 ghost = FALSE, poison = FALSE;
    u32 changed = 0;
    for (u32 i = 0; i < 4; i++)
    {
        u16 move = GetMonData(&mon, MON_DATA_MOVE1 + i);
        ghost |= IsEnemyStabMove(move, TYPE_GHOST);
        poison |= IsEnemyStabMove(move, TYPE_POISON);
        changed += move != MOVE_GROWL;
    }
    EXPECT(ghost || poison);
    EXPECT_EQ(changed, 1 + (ghost && poison));
    EXPECT(!IsEnemyStabMove(MOVE_WEATHER_BALL, TYPE_NORMAL));
    EXPECT(!IsEnemyStabMove(MOVE_SEISMIC_TOSS, TYPE_FIGHTING));
}

TEST("Enemy STAB supplies low-level coverage for every ordinary type")
{
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    FlagSet(FLAG_RUN_RULE_ENEMY_STAB);
    bool8 tested[32] = {0};
    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        if (!gSpeciesInfo[species].baseHP)
            continue;
        u32 type = GetSpeciesType(species, 0);
        u32 other = GetSpeciesType(species, 1);
        if (other != type && other != TYPE_NONE && other != TYPE_MYSTERY && other != TYPE_STELLAR
            && RunRandomizerHash(0x521, TRAINER_ROXANNE_1, 0) % 2)
            type = other;
        if (type == TYPE_NONE || type == TYPE_MYSTERY || type == TYPE_STELLAR || tested[type])
            continue;
        struct Pokemon mon;
        CreateMon(&mon, species, 1, 0, OTID_STRUCT_PLAYER_ID);
        for (u32 i = 0; i < 4; i++)
            SetMonMoveSlot(&mon, MOVE_GROWL, i);
        EnsureEnemyStabMoves(&mon, TRAINER_ROXANNE_1, 0);
        bool32 covered = FALSE;
        for (u32 i = 0; i < 4; i++)
            covered |= IsEnemyStabMove(GetMonData(&mon, MON_DATA_MOVE1 + i), type);
        EXPECT(covered);
        tested[type] = TRUE;
    }
}

TEST("Enemy STAB respects tier boundaries accuracy and strongest available fallbacks for every type")
{
    const u8 levels[] = {1, 9, 10, 19, 20, 29, 30, 39, 40, 49, 50, 100};
    const u8 floors[] = {40, 40, 50, 50, 60, 60, 70, 70, 80, 80, 90, 90};
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    FlagSet(FLAG_RUN_RULE_ENEMY_STAB);
    bool8 tested[32] = {0};
    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        if (!gSpeciesInfo[species].baseHP)
            continue;
        u32 type = GetSpeciesType(species, 0);
        u32 other = GetSpeciesType(species, 1);
        if (other != type && other != TYPE_NONE && other != TYPE_MYSTERY && other != TYPE_STELLAR
            && RunRandomizerHash(0x521, TRAINER_ROXANNE_1, 0) % 2)
            type = other;
        if (type == TYPE_NONE || type == TYPE_MYSTERY || type == TYPE_STELLAR || tested[type])
            continue;
        tested[type] = TRUE;
        for (u32 l = 0; l < ARRAY_COUNT(levels); l++)
        {
            u32 ceiling = levels[l] >= 50 ? 0xFFFF : floors[l] + 30;
            u32 inRange = 0, above = 0xFFFF, below = 0;
            for (u32 move = 1; move < MOVES_COUNT; move++)
            {
                if (!IsEnemyStabMove(move, type) || !IsRandomizerDirectAttack(move)
                    || (gMovesInfo[move].accuracy && gMovesInfo[move].accuracy < 90))
                    continue;
                u32 power = GetRandomizerMovePower(move);
                if (power >= floors[l] && power <= ceiling)
                    inRange++;
                else if (power > ceiling)
                    above = min(above, power);
                else
                    below = max(below, power);
            }
            struct Pokemon mon;
            CreateMon(&mon, species, levels[l], 0, OTID_STRUCT_PLAYER_ID);
            for (u32 i = 0; i < MAX_MON_MOVES; i++)
                SetMonMoveSlot(&mon, MOVE_GROWL, i);
            EnsureEnemyStabMoves(&mon, TRAINER_ROXANNE_1, 0);
            u16 moves[MAX_MON_MOVES];
            bool32 covered = FALSE;
            for (u32 i = 0; i < MAX_MON_MOVES; i++)
            {
                u16 move = moves[i] = GetMonData(&mon, MON_DATA_MOVE1 + i);
                if (!IsEnemyStabMove(move, type))
                    continue;
                covered = TRUE;
                EXPECT(gMovesInfo[move].accuracy == 0 || gMovesInfo[move].accuracy >= 90);
                EXPECT(IsRandomizerDirectAttack(move));
                u32 power = GetRandomizerMovePower(move);
                if (inRange)
                {
                    EXPECT(power >= floors[l]);
                    EXPECT_LE(power, ceiling);
                }
                else
                    EXPECT_EQ(power, above != 0xFFFF ? above : below);
            }
            EXPECT(covered);
            EnsureEnemyStabMoves(&mon, TRAINER_ROXANNE_1, 0);
            for (u32 i = 0; i < MAX_MON_MOVES; i++)
                EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE1 + i), moves[i]);
        }
    }
}

TEST("Enemy STAB upgrades inaccurate attacks but preserves stronger reliable existing coverage")
{
    struct Pokemon mon;
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    FlagSet(FLAG_RUN_RULE_ENEMY_STAB);
    CreateMon(&mon, SPECIES_ARCANINE, 5, 0, OTID_STRUCT_PLAYER_ID);
    SetMonMoveSlot(&mon, MOVE_FIRE_BLAST, 0);
    SetMonMoveSlot(&mon, MOVE_BRAVE_BIRD, 1);
    SetMonMoveSlot(&mon, MOVE_GROWL, 2);
    SetMonMoveSlot(&mon, MOVE_TACKLE, 3);
    EnsureEnemyStabMoves(&mon, TRAINER_ROXANNE_1, 0);
    u16 move = GetMonData(&mon, MON_DATA_MOVE1);
    EXPECT(move != MOVE_FIRE_BLAST);
    EXPECT(IsEnemyStabMove(move, TYPE_FIRE));
    EXPECT(gMovesInfo[move].accuracy == 0 || gMovesInfo[move].accuracy >= 90);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE2), MOVE_BRAVE_BIRD);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE3), MOVE_GROWL);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE4), MOVE_TACKLE);
}

TEST("Enemy STAB final tier has no power cap and stays the same after level fifty")
{
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    FlagSet(FLAG_RUN_RULE_ENEMY_STAB);
    bool32 above120 = FALSE;
    for (u32 slot = 0; slot < 64; slot++)
    {
        struct Pokemon mon;
        u16 at50;
        for (u32 level = 50; level <= 100; level += 50)
        {
            CreateMon(&mon, SPECIES_ARCANINE, level, 0, OTID_STRUCT_PLAYER_ID);
            for (u32 i = 0; i < MAX_MON_MOVES; i++)
                SetMonMoveSlot(&mon, MOVE_GROWL, i);
            EnsureEnemyStabMoves(&mon, TRAINER_ROXANNE_1, slot);
            u16 move = GetMonData(&mon, MON_DATA_MOVE1);
            EXPECT(GetRandomizerMovePower(move) >= 90);
            above120 |= GetRandomizerMovePower(move) > 120;
            if (level == 50)
                at50 = move;
            else
                EXPECT_EQ(move, at50);
        }
    }
    EXPECT(above120);
}

TEST("Enemy STAB independently varies guaranteed type and optional second coverage with stable retries")
{
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    FlagSet(FLAG_RUN_RULE_ENEMY_STAB);
    u32 fireOnly = 0, flyingOnly = 0, both = 0;
    for (u32 trainer = 1; trainer <= 128; trainer++)
    {
        struct Pokemon mon;
        CreateMon(&mon, SPECIES_CHARIZARD, 30, 0, OTID_STRUCT_PLAYER_ID);
        for (u32 i = 0; i < MAX_MON_MOVES; i++)
            SetMonMoveSlot(&mon, MOVE_GROWL, i);
        EnsureEnemyStabMoves(&mon, trainer, 0);
        u16 moves[MAX_MON_MOVES];
        bool32 fire = FALSE, flying = FALSE;
        u32 changed = 0;
        for (u32 i = 0; i < MAX_MON_MOVES; i++)
        {
            moves[i] = GetMonData(&mon, MON_DATA_MOVE1 + i);
            fire |= IsEnemyStabMove(moves[i], TYPE_FIRE);
            flying |= IsEnemyStabMove(moves[i], TYPE_FLYING);
            changed += moves[i] != MOVE_GROWL;
        }
        EXPECT(fire || flying);
        EXPECT_EQ(changed, 1 + (fire && flying));
        fireOnly += fire && !flying;
        flyingOnly += flying && !fire;
        both += fire && flying;
        EnsureEnemyStabMoves(&mon, trainer, 0);
        for (u32 i = 0; i < MAX_MON_MOVES; i++)
            EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE1 + i), moves[i]);
        for (u32 i = 0; i < MAX_MON_MOVES; i++)
            SetMonMoveSlot(&mon, MOVE_GROWL, i);
        EnsureEnemyStabMoves(&mon, trainer, 0);
        for (u32 i = 0; i < MAX_MON_MOVES; i++)
            EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE1 + i), moves[i]);
    }
    EXPECT(fireOnly > 10 && fireOnly < 55);
    EXPECT(flyingOnly > 10 && flyingOnly < 55);
    EXPECT(both > 35 && both < 95);
}

TEST("Enemy STAB retains natural secondary coverage on failed rolls and preserves rolls across save load")
{
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    FlagSet(FLAG_RUN_RULE_ENEMY_STAB);
    u32 trainer;
    for (trainer = 1; trainer < 1000; trainer++)
        if (RunRandomizerHash(0x521, trainer, 0) % 2 == 0 && RunRandomizerHash(0x522, trainer, 0) % 2 == 0)
            break;
    EXPECT(trainer < 1000);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    u16 moves[MAX_MON_MOVES];
    for (u32 attempt = 0; attempt < 2; attempt++)
    {
        struct Pokemon mon;
        CreateMon(&mon, SPECIES_CHARIZARD, 50, 0, OTID_STRUCT_PLAYER_ID);
        SetMonMoveSlot(&mon, MOVE_EMBER, 0);
        SetMonMoveSlot(&mon, MOVE_GUST, 1);
        SetMonMoveSlot(&mon, MOVE_GROWL, 2);
        SetMonMoveSlot(&mon, MOVE_TACKLE, 3);
        EnsureEnemyStabMoves(&mon, trainer, 0);
        EXPECT(GetMonData(&mon, MON_DATA_MOVE1) != MOVE_EMBER);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE2), MOVE_GUST);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE3), MOVE_GROWL);
        for (u32 i = 0; i < MAX_MON_MOVES; i++)
            if (!attempt)
                moves[i] = GetMonData(&mon, MON_DATA_MOVE1 + i);
            else
                EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE1 + i), moves[i]);
        gSaveBlock2Ptr->playerTrainerId[0] ^= 0xFF;
        EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    }
}
