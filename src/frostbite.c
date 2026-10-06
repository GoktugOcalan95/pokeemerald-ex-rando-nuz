#include "global.h"
#include "event_data.h"
#include "frostbite.h"
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
        return COMPOUND_STRING(FROSTBITE_DESC_ICE_PUNCH);
    case MOVE_ICE_BEAM:
        return COMPOUND_STRING(FROSTBITE_DESC_ICE_BEAM);
    case MOVE_BLIZZARD:
        return COMPOUND_STRING(FROSTBITE_DESC_BLIZZARD);
#if B_UPDATED_MOVE_DATA >= GEN_2
    case MOVE_TRI_ATTACK:
        return COMPOUND_STRING(FROSTBITE_DESC_TRI_ATTACK);
#endif
    case MOVE_POWDER_SNOW:
        return COMPOUND_STRING(FROSTBITE_DESC_POWDER_SNOW);
    case MOVE_ICE_FANG:
        return COMPOUND_STRING(FROSTBITE_DESC_ICE_FANG);
#if B_UPDATED_MOVE_DATA < GEN_CHAMPIONS
    case MOVE_FREEZE_DRY:
        return COMPOUND_STRING(FROSTBITE_DESC_FREEZE_DRY);
#endif
    case MOVE_FREEZING_GLARE:
        return COMPOUND_STRING(FROSTBITE_DESC_FREEZING_GLARE);
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
        return COMPOUND_STRING(FROSTBITE_DESC_ICE_HEAL);
    case ITEM_ASPEAR_BERRY:
        return COMPOUND_STRING(FROSTBITE_DESC_ASPEAR_BERRY);
    case ITEM_TM13:
        return COMPOUND_STRING(FROSTBITE_DESC_TM_ICE_BEAM);
    case ITEM_TM14:
        return COMPOUND_STRING(FROSTBITE_DESC_TM_BLIZZARD);
    default:
        return description;
    }
}
