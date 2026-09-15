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
        if (learnset[i] == move)
            return TRUE;
    }

    return FALSE;
}

static bool32 IsDuplicateLearnsetMove(const u16 *learnset, u32 index)
{
    for (u32 i = 0; i < index; i++)
    {
        if (learnset[i] == learnset[index])
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

static bool32 IsMoveInLevelUpLearnset(const struct LevelUpMove *learnset, enum Move move)
{
    if (learnset == NULL)
        return FALSE;
    for (u32 i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
        if (learnset[i].move == move)
            return TRUE;
    return FALSE;
}

bool32 CanPlayerLearnTeachableMove(enum Species species, enum Move move)
{
    if (!IsValidPlayerSpecies(species) || move == MOVE_NONE || move >= MOVES_COUNT)
        return FALSE;
    if (CanLearnTeachableMove(species, move))
        return TRUE;
    if (!IsMachineMove(move) && !IsTutorMove(move))
        return FALSE;
    if (FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY))
        return TRUE;

    return IsMoveInLearnset(GetSpeciesEggMoves(species), move)
        || IsMoveInLevelUpLearnset(gSpeciesInfo[species].levelUpLearnset, move)
        || IsMoveInLevelUpLearnset(GetSpeciesLevelUpLearnset(species), move);
}

static enum Move GetPlayerTeachableMoveInternal(enum Species species, u32 index, u32 *count)
{
    const u16 *learnset = GetSpeciesTeachableLearnset(species);
    *count = 0;

    for (u32 i = 0; learnset[i] != MOVE_UNAVAILABLE; i++)
    {
        if (IsDuplicateLearnsetMove(learnset, i))
            continue;
        if ((*count)++ == index)
            return learnset[i];
    }

    for (u32 i = 0; i < NUM_ALL_MACHINES; i++)
    {
        enum Move move = GetTMHMMoveId(i + 1);

        if (move == MOVE_NONE || IsMoveInLearnset(learnset, move) || IsDuplicateMachineMove(move, i)
            || !CanPlayerLearnTeachableMove(species, move))
            continue;
        if ((*count)++ == index)
            return move;
    }

    for (u32 i = 0; GetTutorMove(i) != MOVE_UNAVAILABLE; i++)
    {
        enum Move move = GetTutorMove(i);

        if (IsMoveInLearnset(learnset, move) || IsMachineMove(move) || IsDuplicateTutorMove(move, i)
            || !CanPlayerLearnTeachableMove(species, move))
            continue;
        if ((*count)++ == index)
            return move;
    }

    return MOVE_NONE;
}

u32 GetPlayerTeachableMoveCount(enum Species species)
{
    u32 count;

    if (!IsValidPlayerSpecies(species))
        return 0;
    GetPlayerTeachableMoveInternal(species, (u32)-1, &count);
    return count;
}

enum Move GetPlayerTeachableMove(enum Species species, u32 index)
{
    u32 count;

    if (!IsValidPlayerSpecies(species))
        return MOVE_NONE;
    return GetPlayerTeachableMoveInternal(species, index, &count);
}
