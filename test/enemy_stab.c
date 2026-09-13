#include "global.h"
#include "enemy_stab.h"
#include "event_data.h"
#include "move.h"
#include "move_randomizer.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/flags.h"

TEST("Enemy STAB adds distinct natural types with minimal replacements and stable PP")
{
    struct Pokemon mon;
    FlagSet(FLAG_RUN_RULE_TRAINERS);
    FlagSet(FLAG_RUN_RULE_ENEMY_STAB);
    CreateMon(&mon, SPECIES_CHARIZARD, 5, 0, OTID_STRUCT_PLAYER_ID);
    SetMonMoveSlot(&mon, MOVE_EMBER, 0);
    SetMonMoveSlot(&mon, MOVE_GROWL, 1);
    SetMonMoveSlot(&mon, MOVE_TACKLE, 2);
    SetMonMoveSlot(&mon, MOVE_SCRATCH, 3);
    EnsureEnemyStabMoves(&mon, TRAINER_ROXANNE_1, 0);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE1), MOVE_EMBER);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE3), MOVE_TACKLE);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE4), MOVE_SCRATCH);
    u16 move = GetMonData(&mon, MON_DATA_MOVE2);
    EXPECT(IsEnemyStabMove(move, TYPE_FLYING));
    EXPECT(IsRandomizerDirectAttack(move));
    EXPECT_LE(GetRandomizerMovePower(move), 40);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_PP2), GetMovePP(move));
    EnsureEnemyStabMoves(&mon, TRAINER_ROXANNE_1, 0);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE2), move);
}

TEST("Enemy STAB covers both missing types and leaves disabled rules unchanged")
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
    EXPECT(ghost);
    EXPECT(poison);
    EXPECT_EQ(changed, 2);
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
