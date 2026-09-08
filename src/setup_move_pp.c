#include "global.h"
#include "event_data.h"
#include "setup_move_pp.h"
#include "constants/flags.h"

bool32 IsSetupMovePPLimited(enum Move move)
{
#if IS_FRLG
    return FALSE;
#else
    switch (move)
    {
    case MOVE_AGILITY:
    case MOVE_AUTOTOMIZE:
    case MOVE_CLANGOROUS_SOUL:
    case MOVE_COTTON_GUARD:
    case MOVE_DRAGON_DANCE:
    case MOVE_MINIMIZE:
    case MOVE_NASTY_PLOT:
    case MOVE_QUIVER_DANCE:
    case MOVE_ROCK_POLISH:
    case MOVE_SHELL_SMASH:
    case MOVE_SHIFT_GEAR:
    case MOVE_SWORDS_DANCE:
    case MOVE_TAIL_GLOW:
    case MOVE_TIDY_UP:
    case MOVE_VICTORY_DANCE:
        return FlagGet(FLAG_RUN_RULE_SETUP_MOVE_PP);
    default:
        return FALSE;
    }
#endif
}
