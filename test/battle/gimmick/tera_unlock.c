#include "global.h"
#include "test/battle.h"
#include "battle_util.h"
#include "battle_terastal.h"

SINGLE_BATTLE_TEST("Tera shards gate player eligibility while enemy eligibility stays separate")
{
    bool32 unlocked;
    PARAMETRIZE { unlocked = FALSE; }
    PARAMETRIZE { unlocked = TRUE; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { TeraType(TYPE_PSYCHIC); TeraUnlocked(unlocked); }
        OPPONENT(SPECIES_WOBBUFFET) { TeraType(TYPE_PSYCHIC); TeraUnlocked(FALSE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); MOVE(opponent, MOVE_SPLASH); }
    } THEN {
        EXPECT_EQ(CanTerastallize(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)), unlocked);
        EXPECT(CanTerastallize(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT)));
    }
}
