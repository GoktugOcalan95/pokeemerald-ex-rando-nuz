#include "global.h"
#include "event_data.h"
#include "constants/vars.h"
#include "move.h"
#include "move_randomizer.h"
#include "run_randomizer.h"
#include "constants/characters.h"

bool32 IsRandomizerMoveAllowed(u16 move)
{
    return move > MOVE_NONE && move < MOVES_COUNT && move != MOVE_STRUGGLE
        && gMovesInfo[move].effect != EFFECT_PLACEHOLDER && gMovesInfo[move].pp != 0
        && gMovesInfo[move].name != NULL && gMovesInfo[move].name[0] != EOS;
}

static bool32 RequiresExtraTurn(u16 move)
{
    if (gBattleMoveEffects[gMovesInfo[move].effect].twoTurnEffect)
        return TRUE;
    for (u32 i = 0; i < gMovesInfo[move].numAdditionalEffects; i++)
        if (gMovesInfo[move].additionalEffects[i].moveEffect == MOVE_EFFECT_RECHARGE)
            return TRUE;
    return FALSE;
}

bool32 IsRandomizerDirectAttack(u16 move)
{
    if (!IsRandomizerMoveAllowed(move) || gMovesInfo[move].category == DAMAGE_CATEGORY_STATUS
        || gMovesInfo[move].explosion || RequiresExtraTurn(move))
        return FALSE;
    switch (gMovesInfo[move].effect)
    {
    case EFFECT_FUTURE_SIGHT:
    case EFFECT_OHKO:
    case EFFECT_FIXED_PERCENT_DAMAGE:
    case EFFECT_FIXED_HP_DAMAGE:
    case EFFECT_LEVEL_DAMAGE:
    case EFFECT_PSYWAVE:
    case EFFECT_REFLECT_DAMAGE:
    case EFFECT_BIDE:
    case EFFECT_ENDEAVOR:
    case EFFECT_FINAL_GAMBIT:
    case EFFECT_DREAM_EATER:
    case EFFECT_SNORE:
    case EFFECT_SPIT_UP:
    case EFFECT_FOCUS_PUNCH:
    case EFFECT_SUCKER_PUNCH:
    case EFFECT_LAST_RESORT:
    case EFFECT_BELCH:
    case EFFECT_FLING:
    case EFFECT_NATURAL_GIFT:
    case EFFECT_SYNCHRONOISE:
    case EFFECT_FAIL_IF_NOT_ARG_TYPE:
    case EFFECT_HYPERSPACE_FURY:
    case EFFECT_AURA_WHEEL:
    case EFFECT_POLTERGEIST:
    case EFFECT_SHELL_TRAP:
    case EFFECT_UPPER_HAND:
    case EFFECT_STEEL_ROLLER:
    case EFFECT_FIRST_TURN_ONLY:
    case EFFECT_SKY_DROP:
        return FALSE;
    default:
        return TRUE;
    }
}

static u32 ExpectedPower(u16 move)
{
    const struct MoveInfo *info = &gMovesInfo[move];
    u32 power = info->power;
    // Use neutral conditions: full HP, no boosts, equal speed/weight, no item bonuses.
    switch (info->effect)
    {
    case EFFECT_FLAIL: power = 20; break;
    case EFFECT_RETURN: power = 28; break;
    case EFFECT_FRUSTRATION: power = 74; break;
    case EFFECT_MAGNITUDE: power = 71; break;
    case EFFECT_PRESENT: power = 52; break;
    case EFFECT_FICKLE_BEAM: power = power * 13 / 10; break;
    case EFFECT_POWER_BASED_ON_USER_HP: break;
    case EFFECT_POWER_BASED_ON_TARGET_HP: break;
    case EFFECT_LOW_KICK: power = 60; break;
    case EFFECT_HEAT_CRASH: power = 40; break;
    case EFFECT_ELECTRO_BALL: power = 60; break;
    case EFFECT_GYRO_BALL: power = 26; break;
    case EFFECT_STORED_POWER: power = 20; break;
    case EFFECT_PUNISHMENT: power = 60; break;
    case EFFECT_TRUMP_CARD: power = 40; break;
    case EFFECT_BEAT_UP: power = 10; break;
    case EFFECT_FLING: power = 30; break;
    case EFFECT_NATURAL_GIFT: power = 80; break;
    case EFFECT_SPIT_UP: power = 100; break;
    case EFFECT_RAGE_FIST:
    case EFFECT_LAST_RESPECTS: power = 50; break;
    case EFFECT_LEVEL_DAMAGE:
    case EFFECT_PSYWAVE:
    case EFFECT_FIXED_PERCENT_DAMAGE:
    case EFFECT_ENDEAVOR:
    case EFFECT_FINAL_GAMBIT:
    case EFFECT_REFLECT_DAMAGE:
    case EFFECT_BIDE: power = 50; break;
    case EFFECT_FIXED_HP_DAMAGE: power = info->argument.fixedDamage; break;
    case EFFECT_OHKO: power = 150; break;
    default: break;
    }
    if (info->multiHit)
        return power * 31 / 10;
    if (info->effect == EFFECT_POPULATION_BOMB || info->effect == EFFECT_TRIPLE_KICK)
    {
        u32 total = 0;
        u32 chance = 10000;
        u32 hits = info->effect == EFFECT_POPULATION_BOMB ? 10 : 3;
        for (u32 hit = 1; hit <= hits; hit++)
        {
            total += power * chance * (info->effect == EFFECT_TRIPLE_KICK ? hit : 1);
            chance = chance * (info->accuracy ? info->accuracy : 100) / 100;
        }
        return total / 10000;
    }
    return power * max(1, info->strikeCount);
}

u32 GetRandomizerMovePower(u16 move)
{
    if (!IsRandomizerMoveAllowed(move) || gMovesInfo[move].category == DAMAGE_CATEGORY_STATUS)
        return 0;
    u32 power = ExpectedPower(move) * (gMovesInfo[move].accuracy ? gMovesInfo[move].accuracy : 100);
    if (gMovesInfo[move].effect == EFFECT_BIDE)
        return power / 300;
    // Preserve burst value while discounting the extra turn.
    return RequiresExtraTurn(move) ? power * 3 / 400 : power / 100;
}

bool32 IsRandomizerGoodAttack(u16 move)
{
    if (!IsRandomizerDirectAttack(move) || (gMovesInfo[move].accuracy != 0 && gMovesInfo[move].accuracy < 90))
        return FALSE;
    switch (gMovesInfo[move].effect)
    {
    case EFFECT_LOW_KICK:
    case EFFECT_HEAT_CRASH:
    case EFFECT_ELECTRO_BALL:
    case EFFECT_GYRO_BALL:
    case EFFECT_FRUSTRATION:
    case EFFECT_POWER_BASED_ON_TARGET_HP:
        return FALSE;
    default:
        return ExpectedPower(move) >= 60;
    }
}

u32 GetRandomizerGoodMoveChance(void)
{
    return min(100, VarGet(VAR_RUN_RULE_GOOD_MOVE_CHANCE));
}

u16 ChooseRandomizerMove(u32 domain, u32 source, u32 slot, const bool8 *used)
{
    bool32 good = RunRandomizerHash(domain ^ 0x80000000, source, slot) % 100 < GetRandomizerGoodMoveChance();
    for (u32 attempt = 0; attempt < 2; attempt++, good = FALSE)
    {
        u32 count = 0;
        for (u32 move = 1; move < MOVES_COUNT; move++)
            if (IsRandomizerMoveAllowed(move) && (used == NULL || !used[move]) && (!good || IsRandomizerGoodAttack(move)))
                count++;
        if (count == 0)
            continue;
        u32 choice = RunRandomizerHash(domain, source, slot) % count;
        for (u32 move = 1; move < MOVES_COUNT; move++)
            if (IsRandomizerMoveAllowed(move) && (used == NULL || !used[move]) && (!good || IsRandomizerGoodAttack(move)) && choice-- == 0)
                return move;
    }
    return MOVE_NONE;
}
