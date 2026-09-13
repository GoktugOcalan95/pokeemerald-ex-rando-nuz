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

static u32 GetStabMinimumPower(u32 level)
{
    static const u8 minimums[] = {40, 50, 60, 70, 80, 90};
    return minimums[min(level / 10, ARRAY_COUNT(minimums) - 1)];
}

static bool32 IsEligibleStabAttack(u16 move, u32 type)
{
    return IsEnemyStabMove(move, type) && IsRandomizerDirectAttack(move)
        && (gMovesInfo[move].accuracy == 0 || gMovesInfo[move].accuracy >= 90);
}

static u16 ChooseStabMove(u32 type, u32 level, u16 trainerId, u32 partySlot)
{
    u32 floor = GetStabMinimumPower(level);
    u32 ceiling = level >= 50 ? 0xFFFF : floor + 30;
    u32 weakestAbove = 0xFFFF, strongestBelow = 0, inRange = 0;
    for (u32 move = 1; move < MOVES_COUNT; move++)
    {
        if (!IsEligibleStabAttack(move, type))
            continue;
        u32 power = GetRandomizerMovePower(move);
        if (power < floor)
            strongestBelow = max(strongestBelow, power);
        else if (power > ceiling)
            weakestAbove = min(weakestAbove, power);
        else
            inRange++;
    }
    if (!inRange)
        floor = ceiling = weakestAbove != 0xFFFF ? weakestAbove : strongestBelow;
    for (u32 pass = 0; pass < 2; pass++)
    {
        u32 count = 0;
        for (u32 move = 1; move < MOVES_COUNT; move++)
            if (IsEligibleStabAttack(move, type) && GetRandomizerMovePower(move) >= floor
                && GetRandomizerMovePower(move) <= ceiling && (pass || IsRandomizerGoodAttack(move)))
                count++;
        if (!count)
            continue;
        u32 choice = RunRandomizerHash(0x520, trainerId, partySlot * 32 + type) % count;
        for (u32 move = 1; move < MOVES_COUNT; move++)
            if (IsEligibleStabAttack(move, type) && GetRandomizerMovePower(move) >= floor
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
    bool32 dualType = types[1] != types[0] && types[1] != TYPE_NONE
        && types[1] != TYPE_MYSTERY && types[1] != TYPE_STELLAR;
    if (dualType)
    {
        if (RunRandomizerHash(0x521, trainerId, partySlot) % 2)
        {
            u8 type = types[0];
            types[0] = types[1];
            types[1] = type;
        }
        covered[1] = RunRandomizerHash(0x522, trainerId, partySlot) % 2 == 0;
    }
    u16 replacements[2] = {MOVE_NONE, MOVE_NONE};
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
        moves[i] = GetMonData(mon, MON_DATA_MOVE1 + i);
    for (u32 t = 0; t < ARRAY_COUNT(types); t++)
    {
        if (types[t] == TYPE_NONE || types[t] == TYPE_MYSTERY || types[t] == TYPE_STELLAR || (t && types[t] == types[0]))
        {
            covered[t] = TRUE;
            continue;
        }
        if (covered[t])
        {
            // Keep naturally present coverage even when its extra STAB roll fails.
            for (u32 i = 0; i < MAX_MON_MOVES; i++)
                if (IsEnemyStabMove(moves[i], types[t]))
                {
                    keep[i] = TRUE;
                    break;
                }
            continue;
        }
        replacements[t] = ChooseStabMove(types[t], level, trainerId, partySlot);
        u32 minimum = GetStabMinimumPower(level);
        // Retain the best available coverage when a type cannot reach its tier.
        if (replacements[t] != MOVE_NONE)
            minimum = min(minimum, GetRandomizerMovePower(replacements[t]));
        for (u32 i = 0; i < MAX_MON_MOVES; i++)
            if (IsEligibleStabAttack(moves[i], types[t]) && GetRandomizerMovePower(moves[i]) >= minimum)
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
        u16 move = replacements[t];
        if (move == MOVE_NONE)
            continue;
        u32 slot = MAX_MON_MOVES;
        for (u32 i = 0; i < MAX_MON_MOVES; i++)
            if (!keep[i] && IsEnemyStabMove(moves[i], types[t])
                && (slot == MAX_MON_MOVES || GetRandomizerMovePower(moves[i]) < GetRandomizerMovePower(moves[slot])))
                slot = i;
        if (slot == MAX_MON_MOVES)
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
