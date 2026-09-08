#ifndef GUARD_FROSTBITE_H
#define GUARD_FROSTBITE_H

#include "constants/abilities.h"
#include "constants/items.h"
#include "constants/moves.h"

bool32 IsFrostbiteEnabled(void);
const u8 *GetFrostbiteMoveDescription(enum Move move, const u8 *description);
const u8 *GetFrostbiteItemDescription(enum Item item, const u8 *description);
const u8 *GetAbilityDescription(enum Ability ability);

#endif // GUARD_FROSTBITE_H
