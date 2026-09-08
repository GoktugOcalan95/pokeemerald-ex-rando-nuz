#include "global.h"
#include "event_data.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("Setup move PP: both battlers spend their only PP and Leppa Berry restores it")
{
    GIVEN {
        FlagSet(FLAG_RUN_RULE_SETUP_MOVE_PP);
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_SWORDS_DANCE); Item(ITEM_LEPPA_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_AGILITY); }
    } WHEN {
        TURN { MOVE(player, MOVE_SWORDS_DANCE); MOVE(opponent, MOVE_AGILITY); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_AGILITY, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SWORDS_DANCE, player);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, player);
    } THEN {
        EXPECT_EQ(player->pp[0], 1);
        EXPECT_EQ(opponent->pp[0], 0);
        FlagClear(FLAG_RUN_RULE_SETUP_MOVE_PP);
    }
}
