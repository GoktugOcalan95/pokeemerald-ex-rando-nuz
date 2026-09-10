#ifndef GUARD_BOSS_REWARDS_H
#define GUARD_BOSS_REWARDS_H

#include "constants/items.h"

void BossRewards_RecordVictory(u16 trainerId);
enum Item BossRewards_TryDeliver(void);
bool32 BossRewards_TryStartScript(void);
bool32 IsAbilityCustomizationItem(enum Item item);

#endif
