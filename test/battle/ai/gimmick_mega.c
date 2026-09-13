#include "global.h"
#include "test/battle.h"
#include "battle_ai_util.h"

AI_SINGLE_BATTLE_TEST("AI uses Mega Evolution")
{
    u64 flags;
    PARAMETRIZE { flags = AI_FLAG_CHECK_BAD_MOVE; }
    PARAMETRIZE { flags = AI_FLAG_BASIC_TRAINER; }
    PARAMETRIZE { flags = AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT; }
    GIVEN {
        AI_FLAGS(flags);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_VENUSAUR) { Item(ITEM_VENUSAURITE); Moves(MOVE_SLUDGE_BOMB); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_SLUDGE_BOMB, gimmick: GIMMICK_MEGA); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_MEGA_EVOLUTION, opponent);
    } THEN {
        EXPECT_EQ(opponent->species, SPECIES_VENUSAUR_MEGA);
    }
}

