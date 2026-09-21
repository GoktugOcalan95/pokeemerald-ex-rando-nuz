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

static bool32 CanPlayerLearnOfferedMove(enum Species species, enum Move move)
{
    return FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY)
        || IsSpeciesCompatibleWithMove(species, move)
        || (FlagGet(FLAG_RUN_RULE_LEARNSETS)
            && IsMoveInLevelUpLearnset(GetSpeciesLevelUpLearnset(species), move));
}

bool32 CanPlayerLearnTeachableMove(enum Species species, enum Move move)
{
    if (!IsValidPlayerSpecies(species) || move <= MOVE_NONE || move >= MOVES_COUNT || move == MOVE_STRUGGLE)
        return FALSE;
    if (!IsMachineMove(move) && !IsTutorMove(move))
        return FALSE;
    return CanPlayerLearnOfferedMove(species, move);
}

static enum Move GetPlayerTeachableMoveInternal(enum Species species, u32 index, u32 *count)
{
    *count = 0;

    for (u32 i = 0; i < NUM_ALL_MACHINES; i++)
    {
        enum Move move = GetTMHMMoveId(i + 1);

        if (move == MOVE_NONE || IsDuplicateMachineMove(move, i)
            || !CanPlayerLearnOfferedMove(species, move))
            continue;
        if ((*count)++ == index)
            return move;
    }

    for (u32 i = 0; GetTutorMove(i) != MOVE_UNAVAILABLE; i++)
    {
        enum Move move = GetTutorMove(i);

        if (IsMachineMove(move) || IsDuplicateTutorMove(move, i)
            || !CanPlayerLearnOfferedMove(species, move))
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
