#ifndef GUARD_FROSTBITE_H
#define GUARD_FROSTBITE_H

#include "constants/items.h"
#include "constants/moves.h"

// Frostbite wording shared by the B_USE_FROSTBITE data entries and the run-rule overrides.
#define FROSTBITE_DESC_ICE_PUNCH         "An icy punch that may\nleave the foe with frostbite."
#define FROSTBITE_DESC_ICE_BEAM          "Blasts the foe with an icy\nbeam. May cause frostbite."
#define FROSTBITE_DESC_BLIZZARD          "Hits the foes with an icy\nstorm. May cause frostbite."
#define FROSTBITE_DESC_TRI_ATTACK        "Fires three types of beams.\nMay burn/parlyz/frostbite."
#define FROSTBITE_DESC_POWDER_SNOW       "Blasts the foes with a snowy\ngust. May cause frostbite."
#define FROSTBITE_DESC_ICE_FANG          "May cause flinching or\nleave the foe with frostbite."
#define FROSTBITE_DESC_FREEZE_DRY        "Super effective on Water-\ntypes. May cause frostbite."
#define FROSTBITE_DESC_FREEZING_GLARE    "Shoots psychic power from\nthe eyes. May frostbite."
#define FROSTBITE_DESC_ICE_HEAL          "Heals Pokémon\nof frostbite."
#define FROSTBITE_DESC_ASPEAR_BERRY      "A held item that\nheals frostbite\nin battle."
#define FROSTBITE_DESC_TM_ICE_BEAM       "Fires an icy cold\nbeam that may\ncause frostbite."
#define FROSTBITE_DESC_TM_BLIZZARD       "A snow-and-wind\nattack that may\ncause frostbite."
#define FROSTBITE_DESC_MAGMA_ARMOR       "Prevents frostbite."

bool32 IsFrostbiteEnabled(void);
const u8 *GetFrostbiteMoveDescription(enum Move move, const u8 *description);
const u8 *GetFrostbiteItemDescription(enum Item item, const u8 *description);

#endif // GUARD_FROSTBITE_H
