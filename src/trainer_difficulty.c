#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "constants/battle_ai.h"
#include "data.h"
#include "event_data.h"
#include "pokemon.h"
#include "run_randomizer.h"
#include "species_randomizer.h"
#include "trainer_difficulty.h"
#include "trainer_randomizer.h"
#include "constants/flags.h"
#include "constants/form_change_types.h"
#include "constants/vars.h"

u32 GetRunTrainerDifficulty(void)
{
    u32 difficulty = VarGet(VAR_RUN_RULE_DIFFICULTY);
    return difficulty <= RUN_TRAINER_UNFAIR ? difficulty : RUN_TRAINER_NORMAL;
}

bool32 IsRunTrainerBoss(u16 trainerId)
{
    switch (trainerId)
    {
    case TRAINER_WALLY_MAUVILLE:
    case TRAINER_BRENDAN_ROUTE_103_TREECKO:
    case TRAINER_BRENDAN_ROUTE_103_TORCHIC:
    case TRAINER_BRENDAN_ROUTE_103_MUDKIP:
    case TRAINER_MAY_ROUTE_103_TREECKO:
    case TRAINER_MAY_ROUTE_103_TORCHIC:
    case TRAINER_MAY_ROUTE_103_MUDKIP:
        return FALSE;
    default: break;
    }
    switch (GetRunTrainer(trainerId)->trainerClass)
    {
    case TRAINER_CLASS_LEADER:
    case TRAINER_CLASS_ELITE_FOUR:
    case TRAINER_CLASS_CHAMPION:
    case TRAINER_CLASS_AQUA_LEADER:
    case TRAINER_CLASS_AQUA_ADMIN:
    case TRAINER_CLASS_MAGMA_LEADER:
    case TRAINER_CLASS_MAGMA_ADMIN:
    case TRAINER_CLASS_RIVAL:
        return TRUE;
    default: return FALSE;
    }
}

u32 GetRunTrainerPartySize(u16 trainerId, u32 originalCount)
{
    const struct Trainer *trainer = GetRunTrainer(trainerId);
    u32 count = originalCount;
    u32 difficulty = GetRunTrainerDifficulty();
    if (FlagGet(FLAG_RUN_RULE_TRAINERS) && difficulty != RUN_TRAINER_NORMAL)
        count = IsRunTrainerBoss(trainerId) ? PARTY_SIZE : count + difficulty;
    count = min(count, PARTY_SIZE);
    if ((gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) && (B_MULTI_HALF_TEAMS || trainer->multiTeamSize == MULTI_TEAM_SIZE_HALF))
        count = min(count, PARTY_SIZE / 2);
    return count;
}

u32 GetRunTrainerLevel(u16 trainerId, u32 level)
{
    u32 difficulty = GetRunTrainerDifficulty();
    if (difficulty == RUN_TRAINER_UNFAIR)
    {
        if (FlagGet(FLAG_IS_CHAMPION))
            level += 5;
        else if (FlagGet(FLAG_BADGE08_GET))
            level += 4;
        else if (FlagGet(FLAG_BADGE06_GET))
            level += 3;
        else if (FlagGet(FLAG_BADGE02_GET))
            level += 2;
        else
            level++;
    }
    else if (difficulty == RUN_TRAINER_HARD)
    {
        if (IsRunTrainerBoss(trainerId))
        {
            const struct Trainer *trainer = GetRunTrainer(trainerId);
            const struct Trainer *source = trainer->overrideTrainer ? GetRunTrainer(trainer->overrideTrainer) : trainer;
            u32 count = trainer->partySize ? trainer->partySize : source->partySize;
            u32 aceLevel = 0;
            for (u32 i = 0; i < count; i++)
                aceLevel = max(aceLevel, source->party[i].lvl);
            if (level < aceLevel)
                level = min(level + 1, aceLevel - 1);
        }
        else
            level++;
    }
    return min(level, MAX_LEVEL);
}

void GetRunTrainerSpecialSlots(u16 trainerId, u32 count, u32 protectedSlot, u32 *legendarySlot, u32 *megaSlot)
{
    u32 difficulty = GetRunTrainerDifficulty();
    bool32 boss = IsRunTrainerBoss(trainerId);
    u32 chance = difficulty == RUN_TRAINER_HARD ? (boss ? 20 : 0)
        : difficulty == RUN_TRAINER_UNFAIR ? (boss ? 50 : 10) : 0;
    u8 available[PARTY_SIZE];
    u32 eligible = 0;
    *legendarySlot = *megaSlot = PARTY_SIZE;
    for (u32 i = 0; i < count; i++)
        if (i != protectedSlot)
            available[eligible++] = i;
    if (!FlagGet(FLAG_RUN_RULE_TRAINERS) || !eligible)
        return;
    if (RunRandomizerHash(0x510, trainerId, 0) % 100 < chance)
    {
        u32 choice = RunRandomizerHash(0x510, trainerId, 1) % eligible;
        *legendarySlot = available[choice];
        available[choice] = available[--eligible];
    }
    if (eligible && RunRandomizerHash(0x511, trainerId, 0) % 100 < chance)
        *megaSlot = available[RunRandomizerHash(0x511, trainerId, 1) % eligible];
}

u16 PickRunTrainerMega(u16 trainerId, u32 slot, u16 *stone)
{
    u32 count = 0;
    for (u32 pass = 0; pass < 2; pass++)
    {
        u32 choice = pass ? RunRandomizerHash(0x512, trainerId, slot) % count : 0;
        for (u32 species = 1; species < NUM_SPECIES; species++)
        {
            if (!IsRandomizerSpeciesEligible(species) || IsRandomizerSpeciesLegendary(species))
                continue;
            const struct FormChange *changes = GetSpeciesFormChanges(species);
            if (changes == NULL)
                continue;
            for (u32 i = 0; changes[i].method != FORM_CHANGE_TERMINATOR; i++)
                if (changes[i].method == FORM_CHANGE_BATTLE_MEGA_EVOLUTION_ITEM
                    && gSpeciesInfo[changes[i].targetSpecies].isMegaEvolution
                    && !IsRandomizerSpeciesLegendary(changes[i].targetSpecies))
                {
                    if (!pass)
                        count++;
                    else if (choice-- == 0)
                    {
                        *stone = changes[i].param1;
                        return species;
                    }
                }
        }
        if (!count)
            break;
    }
    *stone = ITEM_NONE;
    return SPECIES_NONE;
}

bool32 UsesRunTrainerKnowledge(void)
{
    return (FlagGet(FLAG_RUN_RULE_TRAINERS) || GetRunTrainerDifficulty() != RUN_TRAINER_NORMAL)
        && (IsRunTrainerBattle(TRAINER_BATTLE_PARAM.opponentA)
            || ((gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) && IsRunTrainerBattle(TRAINER_BATTLE_PARAM.opponentB)));
}

u64 GetRunTrainerAIFlags(u16 trainerId, u64 authored)
{
    if (!IsRunTrainerBattle(trainerId))
        return authored;
    bool32 boss = IsRunTrainerBoss(trainerId);
    u32 difficulty = GetRunTrainerDifficulty();
    u64 flags = FlagGet(FLAG_RUN_RULE_TRAINERS) ? (boss ? AI_FLAG_BASIC_TRAINER : AI_FLAG_CHECK_BAD_MOVE) : authored;
    if (difficulty == RUN_TRAINER_UNFAIR || (difficulty == RUN_TRAINER_HARD && boss))
        flags |= AI_FLAG_SMART_MON_CHOICES | AI_FLAG_SMART_SWITCHING;
    if (difficulty == RUN_TRAINER_UNFAIR && boss)
        flags |= AI_FLAG_PP_STALL_PREVENTION;
    return flags;
}
