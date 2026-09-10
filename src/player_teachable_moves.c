#include "global.h"
#include "event_data.h"
#include "item.h"
#include "player_teachable_moves.h"
#include "pokemon.h"
#include "constants/flags.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "teaching_randomizer.h"

static bool32 IsValidPlayerSpecies(enum Species species)
{
    return species > SPECIES_NONE && species < NUM_SPECIES && IsSpeciesEnabled(species);
}

static bool32 IsMachineMove(enum Move move)
{
    for (u32 i = 0; i < NUM_ALL_MACHINES; i++)
    {
        if (GetTMHMMoveId(i + 1) == move)
            return TRUE;
    }

    return FALSE;
}

static bool32 IsTutorMove(enum Move move)
{
    for (u32 i = 0; GetTutorMove(i) != MOVE_UNAVAILABLE; i++)
    {
        if (GetTutorMove(i) == move)
            return TRUE;
    }

    return FALSE;
}

static bool32 IsMoveInLearnset(const u16 *learnset, enum Move move)
{
    for (u32 i = 0; learnset[i] != MOVE_UNAVAILABLE; i++)
    {
        if (GetRandomizedTeachingMove(learnset[i]) == move)
            return TRUE;
    }

    return FALSE;
}

static bool32 IsDuplicateLearnsetMove(const u16 *learnset, u32 index)
{
    for (u32 i = 0; i < index; i++)
    {
        if (GetRandomizedTeachingMove(learnset[i]) == GetRandomizedTeachingMove(learnset[index]))
            return TRUE;
    }

    return FALSE;
}

static bool32 IsDuplicateMachineMove(enum Move move, u32 index)
{
    for (u32 i = 0; i < index; i++)
    {
        if (GetTMHMMoveId(i + 1) == move)
            return TRUE;
    }

    return FALSE;
}

static bool32 IsDuplicateTutorMove(enum Move move, u32 index)
{
    for (u32 i = 0; i < index; i++)
    {
        if (GetTutorMove(i) == move)
            return TRUE;
    }

    return FALSE;
}

static enum Move GetFullCompatibilityMove(enum Species species, u32 index)
{
    const u16 *learnset = GetSpeciesTeachableLearnset(species);

    for (u32 i = 0; learnset[i] != MOVE_UNAVAILABLE; i++)
    {
        if (IsDuplicateLearnsetMove(learnset, i)
            || !CanPlayerLearnTeachableMove(species, GetRandomizedTeachingMove(learnset[i])))
            continue;
        if (index-- == 0)
            return GetRandomizedTeachingMove(learnset[i]);
    }

    for (u32 i = 0; i < NUM_ALL_MACHINES; i++)
    {
        enum Move move = GetTMHMMoveId(i + 1);

        if (move == MOVE_NONE || IsMoveInLearnset(learnset, move) || IsDuplicateMachineMove(move, i))
            continue;
        if (index-- == 0)
            return move;
    }

    for (u32 i = 0; GetTutorMove(i) != MOVE_UNAVAILABLE; i++)
    {
        enum Move move = GetTutorMove(i);

        if (IsMoveInLearnset(learnset, move) || IsMachineMove(move) || IsDuplicateTutorMove(move, i))
            continue;
        if (index-- == 0)
            return move;
    }

    return MOVE_NONE;
}

bool32 CanPlayerLearnTeachableMove(enum Species species, enum Move move)
{
    if (!IsValidPlayerSpecies(species) || move == MOVE_NONE || move == MOVE_UNAVAILABLE)
        return FALSE;
    if (CanLearnTeachableMove(species, move))
        return TRUE;
    if (!FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY))
        return FALSE;

    return IsMachineMove(move) || IsTutorMove(move);
}

u32 GetPlayerTeachableMoveCount(enum Species species)
{
    const u16 *learnset;
    u32 count = 0;

    if (!IsValidPlayerSpecies(species))
        return 0;

    learnset = GetSpeciesTeachableLearnset(species);
    if (!FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY))
    {
        for (u32 i = 0; learnset[i] != MOVE_UNAVAILABLE; i++)
            if (!IsDuplicateLearnsetMove(learnset, i)
                && CanPlayerLearnTeachableMove(species, GetRandomizedTeachingMove(learnset[i])))
                count++;
        return count;
    }

    for (u32 i = 0; learnset[i] != MOVE_UNAVAILABLE; i++)
    {
        if (!IsDuplicateLearnsetMove(learnset, i))
            count++;
    }

    for (u32 i = 0; i < NUM_ALL_MACHINES; i++)
    {
        enum Move move = GetTMHMMoveId(i + 1);

        if (move != MOVE_NONE && !IsMoveInLearnset(learnset, move) && !IsDuplicateMachineMove(move, i))
            count++;
    }

    for (u32 i = 0; GetTutorMove(i) != MOVE_UNAVAILABLE; i++)
    {
        enum Move move = GetTutorMove(i);

        if (!IsMoveInLearnset(learnset, move) && !IsMachineMove(move) && !IsDuplicateTutorMove(move, i))
            count++;
    }

    return count;
}

enum Move GetPlayerTeachableMove(enum Species species, u32 index)
{
    const u16 *learnset;

    if (!IsValidPlayerSpecies(species))
        return MOVE_NONE;

    if (FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY))
        return GetFullCompatibilityMove(species, index);

    learnset = GetSpeciesTeachableLearnset(species);
    for (u32 i = 0; learnset[i] != MOVE_UNAVAILABLE; i++)
    {
        if (IsDuplicateLearnsetMove(learnset, i)
            || !CanPlayerLearnTeachableMove(species, GetRandomizedTeachingMove(learnset[i])))
            continue;
        if (index-- == 0)
            return GetRandomizedTeachingMove(learnset[i]);
    }

    return MOVE_NONE;
}
