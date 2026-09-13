#include "global.h"
#include "test/battle.h"
#include "battle_ai_util.h"
#include "battle_ai_main.h"
#include "battle_controllers.h"
#include "battle_gimmick.h"
#include "battle_ai_record.h"
#include "pokemon.h"
#include "event_data.h"
#include "trainer_difficulty.h"
#include "constants/vars.h"

AI_SINGLE_BATTLE_TEST("Trainer AI keeps unrevealed abilities items and PP unknown")
{
    GIVEN {
        VarSet(VAR_RUN_RULE_DIFFICULTY, RUN_TRAINER_HARD);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER);
        PLAYER(SPECIES_JOLTEON) { Ability(ABILITY_VOLT_ABSORB); Item(ITEM_FOCUS_SASH); Moves(MOVE_SPLASH); }
        OPPONENT(SPECIES_CHANSEY) { Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_TACKLE); }
    } THEN {
        EXPECT(UsesRunTrainerKnowledge());
        EXPECT_EQ(AI_DecideKnownAbilityForTurn(B_BATTLER_0), ABILITY_NONE);
        EXPECT_EQ(AI_DecideHoldEffectForTurn(B_BATTLER_0), HOLD_EFFECT_NONE);
        gBattleMons[B_BATTLER_0].pp[0] = 0;
        EXPECT(AI_HasUsableMovePP(B_BATTLER_0, 0));
        SetBattlerAiData(B_BATTLER_0, gAiLogicData);
        EXPECT_EQ(gAiLogicData->items[B_BATTLER_0], ITEM_NONE);
    }
}

AI_SINGLE_BATTLE_TEST("Trainer AI respects publicly revealed Air Balloon")
{
    GIVEN {
        VarSet(VAR_RUN_RULE_DIFFICULTY, RUN_TRAINER_HARD);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER);
        PLAYER(SPECIES_PIKACHU) { Item(ITEM_AIR_BALLOON); Moves(MOVE_SPLASH); }
        OPPONENT(SPECIES_SANDSHREW) { Moves(MOVE_EARTHQUAKE, MOVE_SPLASH); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_SPLASH); }
    } THEN {
        EXPECT_EQ(AI_DecideHoldEffectForTurn(B_BATTLER_0), HOLD_EFFECT_AIR_BALLOON);
        SetBattlerAiData(B_BATTLER_0, gAiLogicData);
        EXPECT_EQ(gAiLogicData->items[B_BATTLER_0], ITEM_AIR_BALLOON);
        RecordItemEffectBattle(B_BATTLER_0, HOLD_EFFECT_NONE);
        EXPECT_EQ(gAiPartyData->mons[B_TRAINER_PLAYER][0].item, ITEM_NONE);
    }
}

AI_SINGLE_BATTLE_TEST("Trainer AI keeps Illusion after an unusual revealed move")
{
    GIVEN {
        VarSet(VAR_RUN_RULE_DIFFICULTY, RUN_TRAINER_HARD);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER);
        PLAYER(SPECIES_ZOROARK) { Ability(ABILITY_ILLUSION); Moves(MOVE_SPLASH); }
        PLAYER(SPECIES_VENUSAUR);
        OPPONENT(SPECIES_ALAKAZAM) { Moves(MOVE_PSYCHIC, MOVE_DARK_PULSE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_PSYCHIC); }
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_PSYCHIC); }
    }
}

u32 Test_PpStallReduction(enum Move move, enum BattlerId attacker, enum BattlerId defender);

AI_SINGLE_BATTLE_TEST("Trainer AI PP stall simulations use remembered types and ignore unseen reserves")
{
    GIVEN {
        VarSet(VAR_RUN_RULE_DIFFICULTY, RUN_TRAINER_UNFAIR);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER);
        PLAYER(SPECIES_RATICATE) { Moves(MOVE_SPLASH); TeraType(TYPE_GHOST); }
        PLAYER(SPECIES_GENGAR);
        OPPONENT(SPECIES_MACHOP) { Moves(MOVE_SPLASH); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_SPLASH); }
    } THEN {
        gAiBattleData->playerStallMons[B_TRAINER_PLAYER][1] = 10;
        EXPECT_EQ(Test_PpStallReduction(MOVE_LOW_KICK, B_BATTLER_1, B_BATTLER_0), 0);
        struct AiPartyMon *known = &gAiPartyData->mons[B_TRAINER_PLAYER][1];
        known->wasSentInBattle = TRUE;
        known->species = SPECIES_RATICATE;
        known->visibleTypes[0] = TYPE_NORMAL;
        known->visibleTypes[1] = TYPE_NORMAL;
        known->visibleTypes[2] = TYPE_MYSTERY;
        SetActiveGimmick(B_BATTLER_0, GIMMICK_TERA);
        EXPECT_EQ(Test_PpStallReduction(MOVE_LOW_KICK, B_BATTLER_1, B_BATTLER_0), 0);
        EXPECT_EQ(GetActiveGimmick(B_BATTLER_0), GIMMICK_TERA);
    }
}
