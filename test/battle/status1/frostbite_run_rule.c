#include "global.h"
#include "event_data.h"
#include "test/battle.h"
#include "constants/flags.h"

SINGLE_BATTLE_TEST("Frostbite run rule replaces freezing moves, Tri Attack and Secret Power")
{
    bool32 enabled = FALSE;
    enum Move move = MOVE_NONE;

    for (u32 rule = FALSE; rule <= TRUE; rule++)
    {
        PARAMETRIZE { enabled = rule; move = MOVE_ICE_PUNCH; }
        PARAMETRIZE { enabled = rule; move = MOVE_ICE_BEAM; }
        PARAMETRIZE { enabled = rule; move = MOVE_BLIZZARD; }
        PARAMETRIZE { enabled = rule; move = MOVE_POWDER_SNOW; }
        PARAMETRIZE { enabled = rule; move = MOVE_ICE_FANG; }
        PARAMETRIZE { enabled = rule; move = MOVE_FREEZING_GLARE; }
        PARAMETRIZE { enabled = rule; move = MOVE_TRI_ATTACK; }
        PARAMETRIZE { enabled = rule; move = MOVE_SECRET_POWER; }
        PARAMETRIZE { enabled = rule; move = MOVE_FREEZE_DRY; }
    }

    GIVEN {
        WITH_CONFIG(B_UPDATED_MOVE_DATA, GEN_9);
        Environment(BATTLE_ENVIRONMENT_ICE);
        if (enabled)
            FlagSet(FLAG_RUN_RULE_FROSTBITE);
        else
            FlagClear(FLAG_RUN_RULE_FROSTBITE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(2); }
    } WHEN {
        TURN { MOVE(player, move, WITH_RNG(RNG_RANDOM_FROM_LIST, 2)); }
    } SCENE {
        HP_BAR(opponent);
        if (enabled) {
            STATUS_ICON(opponent, frostbite: TRUE);
            MESSAGE("The opposing Wobbuffet was hurt by its frostbite!");
            HP_BAR(opponent);
        } else {
            STATUS_ICON(opponent, freeze: TRUE);
        }
    } THEN {
        EXPECT_EQ(opponent->status1, enabled ? STATUS1_FROSTBITE : STATUS1_FREEZE);
        FlagClear(FLAG_RUN_RULE_FROSTBITE);
    }
}

SINGLE_BATTLE_TEST("Frostbite run rule preserves Ice type and Magma Armor immunity")
{
    enum Species species;
    enum Ability ability;

    PARAMETRIZE { species = SPECIES_GLALIE; ability = ABILITY_INNER_FOCUS; }
    PARAMETRIZE { species = SPECIES_CAMERUPT; ability = ABILITY_MAGMA_ARMOR; }

    GIVEN {
        FlagSet(FLAG_RUN_RULE_FROSTBITE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(species) { Ability(ability); }
    } WHEN {
        TURN { MOVE(player, MOVE_ICE_BEAM); }
    } THEN {
        EXPECT_EQ(opponent->status1, STATUS1_NONE);
        FlagClear(FLAG_RUN_RULE_FROSTBITE);
    }
}
