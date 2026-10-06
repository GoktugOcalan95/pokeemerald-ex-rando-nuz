#ifndef GUARD_MOVE_RANDOMIZER_H
#define GUARD_MOVE_RANDOMIZER_H

#include "constants/moves.h"

// One bit per move keeps move sets small enough for the stack.
#define MOVE_BITSET_WORDS DIV_ROUND_UP(MOVES_COUNT, 32)

static inline bool32 IsMoveInBitset(const u32 *set, u32 move)
{
    return (set[move / 32] >> (move % 32)) & 1;
}

static inline void AddMoveToBitset(u32 *set, u32 move)
{
    set[move / 32] |= 1u << (move % 32);
}

u32 GetRandomizerGoodMoveChance(void);
u32 GetRandomizerMovePower(u16 move);
bool32 IsRandomizerDirectAttack(u16 move);
bool32 IsRandomizerGoodAttack(u16 move);
bool32 IsRandomizerMoveAllowed(u16 move);
u16 ChooseRandomizerMove(u32 domain, u32 source, u32 slot, const u32 *used);

#endif
