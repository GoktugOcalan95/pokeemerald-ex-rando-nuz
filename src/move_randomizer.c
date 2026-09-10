#include "global.h"
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

u16 ChooseRandomizerMove(u32 domain, u32 source, u32 slot, const bool8 *used)
{
    u32 count = 0;
    for (u32 move = 1; move < MOVES_COUNT; move++)
        if (IsRandomizerMoveAllowed(move) && (used == NULL || !used[move]))
            count++;
    if (count == 0)
        return MOVE_NONE;
    u32 choice = RunRandomizerHash(domain, source, slot) % count;
    for (u32 move = 1; move < MOVES_COUNT; move++)
        if (IsRandomizerMoveAllowed(move) && (used == NULL || !used[move]) && choice-- == 0)
            return move;
    return MOVE_NONE;
}
