#include "global.h"
#include "enemy_stab.h"
#include "event_data.h"
#include "move.h"
#include "move_randomizer.h"
#include "pokemon.h"
#include "run_randomizer.h"
#include "constants/flags.h"

bool32 IsEnemyStabMove(u16 move, u32 type)
{
    if (!IsRandomizerMoveAllowed(move) || gMovesInfo[move].category == DAMAGE_CATEGORY_STATUS || gMovesInfo[move].type != type)
        return FALSE;
    switch (gMovesInfo[move].effect)
    {
    case EFFECT_HIDDEN_POWER:
    case EFFECT_WEATHER_BALL:
    case EFFECT_TERRAIN_PULSE:
    case EFFECT_REVELATION_DANCE:
    case EFFECT_CHANGE_TYPE_ON_ITEM:
    case EFFECT_NATURAL_GIFT:
    case EFFECT_IVY_CUDGEL:
    case EFFECT_RAGING_BULL:
    case EFFECT_AURA_WHEEL:
    case EFFECT_TERA_BLAST:
    case EFFECT_TERA_STARSTORM:
    case EFFECT_OHKO:
    case EFFECT_FIXED_HP_DAMAGE:
    case EFFECT_FIXED_PERCENT_DAMAGE:
    case EFFECT_LEVEL_DAMAGE:
    case EFFECT_PSYWAVE:
    case EFFECT_REFLECT_DAMAGE:
    case EFFECT_ENDEAVOR:
    case EFFECT_FINAL_GAMBIT:
    case EFFECT_BIDE:
        return FALSE;
    default:
        return TRUE;
    }
}

static u16 ChooseStabMove(u32 type, u32 level, u16 trainerId, u32 partySlot)
{
    u32 ceiling = min(120, max(40, 20 + level * 2));
    u32 weakest = 0xFFFF;
    for (u32 move = 1; move < MOVES_COUNT; move++)
        if (IsEnemyStabMove(move, type) && IsRandomizerDirectAttack(move))
            weakest = min(weakest, GetRandomizerMovePower(move));
    ceiling = max(ceiling, weakest);
    for (u32 pass = 0; pass < 2; pass++)
    {
        u32 count = 0;
        for (u32 move = 1; move < MOVES_COUNT; move++)
            if (IsEnemyStabMove(move, type) && IsRandomizerDirectAttack(move)
                && GetRandomizerMovePower(move) <= ceiling && (pass || IsRandomizerGoodAttack(move)))
                count++;
        if (!count)
            continue;
        u32 choice = RunRandomizerHash(0x520, trainerId, partySlot * 32 + type) % count;
        for (u32 move = 1; move < MOVES_COUNT; move++)
            if (IsEnemyStabMove(move, type) && IsRandomizerDirectAttack(move)
                && GetRandomizerMovePower(move) <= ceiling && (pass || IsRandomizerGoodAttack(move)) && choice-- == 0)
                return move;
    }
    return MOVE_NONE;
}

void EnsureEnemyStabMoves(struct Pokemon *mon, u16 trainerId, u32 partySlot)
{
    if (!FlagGet(FLAG_RUN_RULE_TRAINERS) || !FlagGet(FLAG_RUN_RULE_ENEMY_STAB))
        return;
    u32 species = GetMonData(mon, MON_DATA_SPECIES);
    u32 level = GetMonData(mon, MON_DATA_LEVEL);
    u16 moves[MAX_MON_MOVES];
    bool8 keep[MAX_MON_MOVES] = {0};
    u8 types[2] = {GetSpeciesType(species, 0), GetSpeciesType(species, 1)};
    bool8 covered[2] = {0};
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
        moves[i] = GetMonData(mon, MON_DATA_MOVE1 + i);
    for (u32 t = 0; t < ARRAY_COUNT(types); t++)
    {
        if (types[t] == TYPE_NONE || types[t] == TYPE_MYSTERY || types[t] == TYPE_STELLAR || (t && types[t] == types[0]))
        {
            covered[t] = TRUE;
            continue;
        }
        for (u32 i = 0; i < MAX_MON_MOVES; i++)
            if (IsEnemyStabMove(moves[i], types[t]))
            {
                keep[i] = TRUE;
                covered[t] = TRUE;
                break;
            }
    }
    for (u32 t = 0; t < ARRAY_COUNT(types); t++)
    {
        if (covered[t])
            continue;
        u16 move = ChooseStabMove(types[t], level, trainerId, partySlot);
        if (move == MOVE_NONE)
            continue;
        u32 slot = MAX_MON_MOVES;
        for (u32 i = 0; i < MAX_MON_MOVES; i++)
            if (!keep[i] && (slot == MAX_MON_MOVES || GetRandomizerMovePower(moves[i]) < GetRandomizerMovePower(moves[slot])))
                slot = i;
        if (slot == MAX_MON_MOVES)
            continue;
        SetMonMoveSlot(mon, move, slot);
        moves[slot] = move;
        keep[slot] = TRUE;
    }
}
