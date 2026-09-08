#include "global.h"
#include "event_data.h"
#include "item_use.h"
#include "test/battle.h"

WILD_BATTLE_TEST("Instant catch guarantees critical captures with every ball and preserves ball effects")
{
    enum Item item = ITEM_NONE;
    for (enum PokeBall ball = BALL_STRANGE; ball < POKEBALL_COUNT; ball++)
        PARAMETRIZE { item = gPokeBalls[ball].itemId; }

    GIVEN {
        FlagSet(FLAG_RUN_RULE_INSTANT_CATCH);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_BELDUM) { Level(100); HP(10); Status1(STATUS1_PARALYSIS); }
    } WHEN {
        TURN { USE_ITEM(player, item, WITH_RNG(RNG_BALLTHROW_SHAKE, MAX_u16)); }
    } SCENE {
        ANIMATION(ANIM_TYPE_SPECIAL, B_ANIM_CRITICAL_CAPTURE_THROW);
        NOT ANIMATION(ANIM_TYPE_SPECIAL, B_ANIM_BALL_THROW);
    } THEN {
        struct Pokemon *caught = &gParties[B_TRAINER_PLAYER][1];
        EXPECT_EQ(GetMonData(caught, MON_DATA_SPECIES), SPECIES_BELDUM);
        EXPECT_EQ(GetMonData(caught, MON_DATA_POKEBALL), GetItemSecondaryId(item));
        if (item == ITEM_HEAL_BALL)
        {
            EXPECT_EQ(GetMonData(caught, MON_DATA_HP), GetMonData(caught, MON_DATA_MAX_HP));
            EXPECT_EQ(GetMonData(caught, MON_DATA_STATUS), STATUS1_NONE);
        }
        if (item == ITEM_FRIEND_BALL)
            EXPECT_EQ(GetMonData(caught, MON_DATA_FRIENDSHIP), (B_FRIEND_BALL_MODIFIER >= GEN_8 ? 150 : 200));
        FlagClear(FLAG_RUN_RULE_INSTANT_CATCH);
    }
}

WILD_BATTLE_TEST("Instant catch Off retains failed captures")
{
    GIVEN {
        FlagClear(FLAG_RUN_RULE_INSTANT_CATCH);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_BELDUM) { Level(100); }
    } WHEN {
        TURN { USE_ITEM(player, ITEM_POKE_BALL, WITH_RNG(RNG_BALLTHROW_SHAKE, MAX_u16)); }
    } THEN {
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_SPECIES), SPECIES_NONE);
    }
}
