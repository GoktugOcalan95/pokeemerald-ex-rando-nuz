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
#include "constants/flags.h"
#include "player_teachable_moves.h"

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

bool32 Test_ShouldFailForIllusion(enum Species species, enum BattlerId battler);

AI_SINGLE_BATTLE_TEST("Illusion AI respects full native compatibility and run teaching rules")
{
    GIVEN {
        VarSet(VAR_RUN_RULE_DIFFICULTY, RUN_TRAINER_NORMAL);
        FlagClear(FLAG_RUN_RULE_TRAINERS);
        FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
        FlagClear(FLAG_RUN_RULE_LEARNSETS);
        FlagClear(FLAG_RUN_RULE_TMS_TUTORS);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER);
        PLAYER(SPECIES_ZOROARK) { Ability(ABILITY_ILLUSION); Moves(MOVE_SPLASH); }
        PLAYER(SPECIES_VENUSAUR);
        OPPONENT(SPECIES_ALAKAZAM) { Moves(MOVE_SPLASH); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_SPLASH); }
    } THEN {
        EXPECT(!UsesRunTrainerKnowledge());
        gBattleHistory->abilities[B_BATTLER_0] = ABILITY_NONE;
        for (u32 i = 0; i < MAX_MON_MOVES; i++)
            gBattleHistory->usedMoves[B_BATTLER_0][i] = MOVE_NONE;

        gBattleHistory->usedMoves[B_BATTLER_0][0] = MOVE_THUNDER_FANG;
        EXPECT(!CanLearnTeachableMove(SPECIES_BAXCALIBUR, MOVE_THUNDER_FANG));
        EXPECT(Test_ShouldFailForIllusion(SPECIES_BAXCALIBUR, B_BATTLER_0));

        gBattleHistory->usedMoves[B_BATTLER_0][0] = MOVE_THUNDERBOLT;
        EXPECT(!IsSpeciesCompatibleWithMove(SPECIES_VENUSAUR, MOVE_THUNDERBOLT));
        EXPECT(!Test_ShouldFailForIllusion(SPECIES_VENUSAUR, B_BATTLER_0));
        FlagSet(FLAG_RUN_RULE_FULL_COMPATIBILITY);
        EXPECT(CanPlayerLearnTeachableMove(SPECIES_VENUSAUR, MOVE_THUNDERBOLT));
        EXPECT(Test_ShouldFailForIllusion(SPECIES_VENUSAUR, B_BATTLER_0));

        gBattleHistory->usedMoves[B_BATTLER_0][0] = MOVE_DRAGON_ASCENT;
        EXPECT(!CanPlayerLearnTeachableMove(SPECIES_VENUSAUR, MOVE_DRAGON_ASCENT));
        EXPECT(!Test_ShouldFailForIllusion(SPECIES_VENUSAUR, B_BATTLER_0));
        FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
        FlagSet(FLAG_RUN_RULE_LEARNSETS);
        const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(SPECIES_VENUSAUR);
        bool32 found = FALSE;
        for (u32 i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
            if (!IsSpeciesCompatibleWithMove(SPECIES_VENUSAUR, learnset[i].move))
            {
                found = TRUE;
                gBattleHistory->usedMoves[B_BATTLER_0][0] = learnset[i].move;
                EXPECT(Test_ShouldFailForIllusion(SPECIES_VENUSAUR, B_BATTLER_0));
                break;
            }
        EXPECT(found);
        gBattleHistory->abilities[B_BATTLER_0] = ABILITY_ILLUSION;
        EXPECT(!Test_ShouldFailForIllusion(SPECIES_VENUSAUR, B_BATTLER_0));
        FlagClear(FLAG_RUN_RULE_LEARNSETS);
    }
}
