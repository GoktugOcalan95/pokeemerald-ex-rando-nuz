#include "global.h"
#include "event_data.h"
#include "frostbite.h"
#include "pokemon.h"
#include "constants/flags.h"

bool32 IsFrostbiteEnabled(void)
{
#if IS_FRLG
    return B_USE_FROSTBITE;
#else
    return B_USE_FROSTBITE || FlagGet(FLAG_RUN_RULE_FROSTBITE);
#endif
}

const u8 *GetFrostbiteMoveDescription(enum Move move, const u8 *description)
{
    if (!IsFrostbiteEnabled())
        return description;

    switch (move)
    {
    case MOVE_ICE_PUNCH:
        return COMPOUND_STRING("An icy punch that may\nleave the foe with frostbite.");
    case MOVE_ICE_BEAM:
        return COMPOUND_STRING("Blasts the foe with an icy\nbeam. May cause frostbite.");
    case MOVE_BLIZZARD:
        return COMPOUND_STRING("Hits the foes with an icy\nstorm. May cause frostbite.");
#if B_UPDATED_MOVE_DATA >= GEN_2
    case MOVE_TRI_ATTACK:
        return COMPOUND_STRING("Fires three types of beams.\nMay burn/parlyz/frostbite.");
#endif
    case MOVE_POWDER_SNOW:
        return COMPOUND_STRING("Blasts the foes with a snowy\ngust. May cause frostbite.");
    case MOVE_ICE_FANG:
        return COMPOUND_STRING("May cause flinching or\nleave the foe with frostbite.");
#if B_UPDATED_MOVE_DATA < GEN_CHAMPIONS
    case MOVE_FREEZE_DRY:
        return COMPOUND_STRING("Super effective on Water-\ntypes. May cause frostbite.");
#endif
    case MOVE_FREEZING_GLARE:
        return COMPOUND_STRING("Shoots psychic power from\nthe eyes. May frostbite.");
    default:
        return description;
    }
}

const u8 *GetFrostbiteItemDescription(enum Item item, const u8 *description)
{
    if (!IsFrostbiteEnabled())
        return description;

    switch (item)
    {
    case ITEM_ICE_HEAL:
        return COMPOUND_STRING("Heals Pokémon\nof frostbite.");
    case ITEM_ASPEAR_BERRY:
        return COMPOUND_STRING("A held item that\nheals frostbite\nin battle.");
    case ITEM_TM13:
        return COMPOUND_STRING("Fires an icy cold\nbeam that may\ncause frostbite.");
    case ITEM_TM14:
        return COMPOUND_STRING("A snow-and-wind\nattack that may\ncause frostbite.");
    default:
        return description;
    }
}

const u8 *GetAbilityDescription(enum Ability ability)
{
    if (ability == ABILITY_MAGMA_ARMOR && IsFrostbiteEnabled())
        return COMPOUND_STRING("Prevents frostbite.");
    return gAbilitiesInfo[ability].description;
}
