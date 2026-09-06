#ifndef GUARD_PLAYER_TEACHABLE_MOVES_H
#define GUARD_PLAYER_TEACHABLE_MOVES_H

bool32 CanPlayerLearnTeachableMove(enum Species species, enum Move move);
u32 GetPlayerTeachableMoveCount(enum Species species);
enum Move GetPlayerTeachableMove(enum Species species, u32 index);

extern const u16 gTutorMoves[];

#endif // GUARD_PLAYER_TEACHABLE_MOVES_H
