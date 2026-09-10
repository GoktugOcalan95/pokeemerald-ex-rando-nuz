#include "global.h"
#include "event_data.h"
#include "test/battle.h"
#include "constants/flags.h"

SINGLE_BATTLE_TEST("Ability randomizer Palafin keeps the selected slot through switching and native transformation")
{
    u32 slot;
    PARAMETRIZE { slot = 0; }
    PARAMETRIZE { slot = 1; }
    PARAMETRIZE { slot = 2; }
    GIVEN {
        FlagSet(FLAG_RUN_RULE_ABILITIES);
        PLAYER(SPECIES_PALAFIN_ZERO) { Ability(GetSpeciesAbility(SPECIES_PALAFIN_ZERO, slot)); }
        PLAYER(SPECIES_MAGIKARP) { Ability(ABILITY_RUN_AWAY); }
        OPPONENT(SPECIES_MAGIKARP) { Ability(ABILITY_RUN_AWAY); }
    } WHEN {
        TURN { SWITCH(player, 1); }
        TURN { SWITCH(player, 0); }
    } THEN {
        EXPECT_EQ(player->species, slot == 0 ? SPECIES_PALAFIN_HERO : SPECIES_PALAFIN_ZERO);
        EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_ABILITY_NUM), slot);
        EXPECT_EQ(player->ability, GetSpeciesAbility(player->species, slot));
    }
}

SINGLE_BATTLE_TEST("Ability randomizer weather upgrades require Forecast")
{
    u32 slot;
    PARAMETRIZE { slot = 0; }
    PARAMETRIZE { slot = 1; }
    PARAMETRIZE { slot = 2; }
    GIVEN {
        FlagSet(FLAG_RUN_RULE_ABILITIES);
        PLAYER(SPECIES_CASTFORM) { Ability(GetSpeciesAbility(SPECIES_CASTFORM, slot)); }
        OPPONENT(SPECIES_MAGIKARP) { Ability(ABILITY_RUN_AWAY); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUNNY_DAY); }
    } THEN {
        EXPECT_EQ(player->species, slot == 0 ? SPECIES_CASTFORM_SUNNY : SPECIES_CASTFORM);
        EXPECT_EQ(player->ability, GetSpeciesAbility(player->species, slot));
    }
}

SINGLE_BATTLE_TEST("Ability randomizer Terapagos native slot permits Stellar after temporary ability replacement")
{
    bool32 randomize;
    PARAMETRIZE { randomize = FALSE; }
    PARAMETRIZE { randomize = TRUE; }
    GIVEN {
        if (randomize)
            FlagSet(FLAG_RUN_RULE_ABILITIES);
        PLAYER(SPECIES_TERAPAGOS) { Ability(ABILITY_TERA_SHIFT); }
        OPPONENT(SPECIES_MAGIKARP) { Ability(ABILITY_RUN_AWAY); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_WORRY_SEED); }
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_TERA); }
    } THEN {
        EXPECT_EQ(player->species, SPECIES_TERAPAGOS_STELLAR);
    }
}

SINGLE_BATTLE_TEST("Ability randomizer Terapagos alternative slots do not grant Stellar form")
{
    u32 slot;
    PARAMETRIZE { slot = 1; }
    PARAMETRIZE { slot = 2; }
    GIVEN {
        FlagSet(FLAG_RUN_RULE_ABILITIES);
        PLAYER(SPECIES_TERAPAGOS) { Ability(GetSpeciesAbility(SPECIES_TERAPAGOS, slot)); }
        OPPONENT(SPECIES_MAGIKARP) { Ability(ABILITY_RUN_AWAY); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_TERA); }
    } THEN {
        EXPECT_EQ(player->species, SPECIES_TERAPAGOS);
    }
}
