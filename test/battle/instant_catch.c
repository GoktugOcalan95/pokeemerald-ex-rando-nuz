#include "global.h"
#include "event_data.h"
#include "item_use.h"
#include "run_setup.h"
#include "constants/vars.h"
#include "test/battle.h"

WILD_BATTLE_TEST("Instant catch guarantees critical captures with every ball and preserves ball effects")
{
    enum Item item = ITEM_NONE;
    for (enum PokeBall ball = BALL_STRANGE; ball < POKEBALL_COUNT; ball++)
        PARAMETRIZE { item = gPokeBalls[ball].itemId; }

    GIVEN {
        VarSet(VAR_RUN_RULE_CATCH_BONUS, CATCH_BONUS_INSTANT);
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
        VarSet(VAR_RUN_RULE_CATCH_BONUS, CATCH_BONUS_NONE);
    }
}

WILD_BATTLE_TEST("Instant catch Off retains failed captures")
{
    GIVEN {
        VarSet(VAR_RUN_RULE_CATCH_BONUS, CATCH_BONUS_NONE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_BELDUM) { Level(100); }
    } WHEN {
        TURN { USE_ITEM(player, ITEM_POKE_BALL, WITH_RNG(RNG_BALLTHROW_SHAKE, MAX_u16)); }
    } THEN {
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_SPECIES), SPECIES_NONE);
    }
}

WILD_BATTLE_TEST("Catch bonus rescues failed throws with ordinary animation")
{
    u32 tier = 0;
    bool32 rescued = FALSE;
    for (u32 i = CATCH_BONUS_LOW; i <= CATCH_BONUS_HIGH; i++)
    {
        PARAMETRIZE { tier = i; rescued = TRUE; }
        PARAMETRIZE { tier = i; rescued = FALSE; }
    }
    GIVEN {
        WITH_CONFIG(B_CRITICAL_CAPTURE_IF_OWNED, GEN_8);
        WITH_CONFIG(B_MISSING_BADGE_CATCH_MALUS, GEN_7);
        VarSet(VAR_RUN_RULE_CATCH_BONUS, tier);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_BELDUM) { Level(100); }
    } WHEN {
        TURN { USE_ITEM(player, ITEM_POKE_BALL,
            WITH_RNG(RNG_BALLTHROW_BONUS, rescued ? 0 : MAX_u16)); }
    } SCENE {
        ANIMATION(ANIM_TYPE_SPECIAL, B_ANIM_BALL_THROW);
    } THEN {
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_SPECIES), rescued ? SPECIES_BELDUM : SPECIES_NONE);
        VarSet(VAR_RUN_RULE_CATCH_BONUS, CATCH_BONUS_NONE);
    }
}
