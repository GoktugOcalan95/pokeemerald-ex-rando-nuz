#ifndef GUARD_MOVE_RANDOMIZER_H
#define GUARD_MOVE_RANDOMIZER_H

bool32 IsRandomizerMoveAllowed(u16 move);
u16 ChooseRandomizerMove(u32 domain, u32 source, u32 slot, const bool8 *used);

#endif
