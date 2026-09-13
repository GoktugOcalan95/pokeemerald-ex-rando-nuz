#ifndef GUARD_ENEMY_STAB_H
#define GUARD_ENEMY_STAB_H

struct Pokemon;
bool32 IsEnemyStabMove(u16 move, u32 type);
void EnsureEnemyStabMoves(struct Pokemon *mon, u16 trainerId, u32 partySlot);

#endif
